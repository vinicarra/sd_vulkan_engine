#include "sde_renderer.h"

namespace sde {

	SdeRenderer::SdeRenderer(SdeWindow& window, SdeDevice& device) : m_SdeWindow(window), m_SdeDevice(device)
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	SdeRenderer::~SdeRenderer()
	{
		freeCommandBuffers();
	}

	vk::CommandBuffer SdeRenderer::beginFrame()
	{
		auto acquireData = m_SdeSwapChain->acquireNextImage();

		if (acquireData.result == vk::Result::eErrorOutOfDateKHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (acquireData.result != vk::Result::eSuccess && acquireData.result != vk::Result::eSuboptimalKHR) {
			throw new std::runtime_error("Failed to acquire swapchain image");
		}

		// Update index
		m_CurrentImageIndex = acquireData.value;

		auto commandBuffer = getCurrentCommandBuffer();
		try {
			commandBuffer.begin(vk::CommandBufferBeginInfo({ vk::CommandBufferUsageFlagBits::eSimultaneousUse }));
		}
		catch (vk::SystemError err) {
			throw new std::runtime_error("Failed to record(begin) command buffer");
		}

		return commandBuffer;
	}

	void SdeRenderer::endFrame()
	{
		// End command buffer
		auto commandBuffer = getCurrentCommandBuffer();
		try {
			commandBuffer.end();
		}
		catch (vk::SystemError err) {
			throw new std::runtime_error("Failed to record(end) command buffer");
		}

		// Submit command
		auto result = m_SdeSwapChain->submitCommandBuffers(&commandBuffer, m_CurrentImageIndex);
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_SdeWindow.hasResized()) {
			m_SdeWindow.resetResized();
			recreateSwapChain();
		}
		else if (result != vk::Result::eSuccess) {
			throw new std::runtime_error("Failed to present image");
		}

		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % SdeSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void SdeRenderer::beginSwapChainRenderPass(vk::CommandBuffer buffer)
	{
		vk::RenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.renderPass = m_SdeSwapChain->getRenderPass();
		renderPassInfo.framebuffer = m_SdeSwapChain->getFramebuffer(m_CurrentImageIndex);
		renderPassInfo.renderArea.setOffset({ static_cast<int32_t>(0), static_cast<int32_t>(0) });
		renderPassInfo.renderArea.extent = m_SdeSwapChain->getSwapChainExtent();

		vk::ClearValue clearColor = { std::array<float, 4>{ 0.16f, 0.74f , 0.75f, 1.0f } };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	}

	void SdeRenderer::endSwapChainRenderPass(vk::CommandBuffer buffer)
	{
		buffer.endRenderPass();
	}

	void SdeRenderer::recreateSwapChain()
	{
		auto extent = m_SdeWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = m_SdeWindow.getExtent();
			glfwWaitEvents();
		}

		m_SdeDevice.device().waitIdle();

		if (m_SdeSwapChain == nullptr) {
			m_SdeSwapChain = std::make_unique<SdeSwapChain>(m_SdeDevice, extent);
		}
		else {
			std::shared_ptr<SdeSwapChain> oldSwapChain = std::move(m_SdeSwapChain);
			m_SdeSwapChain = std::make_unique<SdeSwapChain>(m_SdeDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*m_SdeSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}
	}

	void SdeRenderer::createCommandBuffers()
	{
		m_CommandBuffers.resize(SdeSwapChain::MAX_FRAMES_IN_FLIGHT);

		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = m_SdeDevice.commandPool();
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		try {
			m_CommandBuffers = m_SdeDevice.device().allocateCommandBuffers(allocInfo);
		}
		catch (vk::SystemError err) {
			throw std::runtime_error("Failed to allocate command buffers");
		}
	}

	void SdeRenderer::freeCommandBuffers()
	{
		m_SdeDevice.device().freeCommandBuffers(m_SdeDevice.commandPool(), m_CommandBuffers);
		m_CommandBuffers.clear();
	}

}