#include "sde_swap_chain.h"

namespace sde {

	SdeSwapChain::SdeSwapChain(SdeDevice& device, vk::Extent2D windowExtent) : m_Device(device), m_WindowExtent(windowExtent)
	{
		init();
	}

	SdeSwapChain::SdeSwapChain(SdeDevice& device, vk::Extent2D windowExtent, std::shared_ptr<SdeSwapChain> oldSwapChain) : m_Device(device), m_WindowExtent(windowExtent), m_OldSwapChain(oldSwapChain)
	{
		init();
		m_OldSwapChain = nullptr;
	}

	SdeSwapChain::~SdeSwapChain()
	{
		for (auto imageView : m_SwapChainImageViews) {
			m_Device.device().destroyImageView(imageView);
		}
		m_SwapChainImageViews.clear();

		if (m_SwapChain) {
			m_Device.device().destroySwapchainKHR(m_SwapChain);
			m_SwapChain = nullptr;
		}

		for (auto framebuffer : m_Framebuffers) {
			m_Device.device().destroyFramebuffer(framebuffer);
		}

		m_Device.device().destroyRenderPass(m_RenderPass);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			m_Device.device().destroySemaphore(m_ImageSemaphores[i]);
			m_Device.device().destroySemaphore(m_RenderFinishedSemaphores[i]);
			m_Device.device().destroyFence(m_InFlightFences[i]);
		}
	}

	vk::ResultValue<uint32_t> SdeSwapChain::acquireNextImage()
	{
		m_Device.device().waitForFences(1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

		return m_Device.device().acquireNextImageKHR(m_SwapChain, std::numeric_limits<uint64_t>::max(), m_ImageSemaphores[m_CurrentFrame], nullptr);
	}

	vk::Result SdeSwapChain::submitCommandBuffers(const vk::CommandBuffer* buffers, uint32_t imageIndex)
	{
		m_Device.device().waitForFences(1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

		vk::SubmitInfo submitInfo = {};
		vk::Semaphore waitSemaphores[] = { m_ImageSemaphores[m_CurrentFrame] };
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;

		vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		m_Device.device().resetFences(1, &m_InFlightFences[m_CurrentFrame]);

		// Submit to graphics queue
		try {
			m_Device.graphicsQueue().submit(submitInfo, m_InFlightFences[m_CurrentFrame]);
		}
		catch (vk::SystemError err) {
			throw std::runtime_error("Failed to submit draw command buffer");
		}

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		vk::SwapchainKHR swapChains[] = { m_SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		vk::Result resultPresent;
		try {
			resultPresent = m_Device.presentQueue().presentKHR(presentInfo);
		}
		catch (vk::OutOfDateKHRError err) {
			resultPresent = vk::Result::eErrorOutOfDateKHR;
		}
		
		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		return resultPresent;
	}

	void SdeSwapChain::init()
	{
		createSwapChain();
		createImageViews();
		createRenderPass();
		createFramebuffers();
		createSyncObjects();
	}

	void SdeSwapChain::createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = m_Device.getSwapChainSupport();
		
		auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		auto extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; // Request 1 more to avoid waiting
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo(
			vk::SwapchainCreateFlagsKHR(),
			m_Device.surface(),
			imageCount,
			surfaceFormat.format,
			surfaceFormat.colorSpace,
			extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment
		);

		auto indices = m_Device.findPhysicalQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = m_OldSwapChain == nullptr ? VK_NULL_HANDLE : m_OldSwapChain->m_SwapChain;

		try {
			m_SwapChain = m_Device.device().createSwapchainKHR(createInfo);
		}
		catch (vk::SystemError& err) {
			throw std::runtime_error("Failed to create swapchain");
		}

		m_SwapChainImages = m_Device.device().getSwapchainImagesKHR(m_SwapChain);
		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	void SdeSwapChain::createImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
			vk::ImageViewCreateInfo createInfo = {};
			createInfo.image = m_SwapChainImages[i];
			createInfo.viewType = vk::ImageViewType::e2D;
			createInfo.format = m_SwapChainImageFormat;

			createInfo.components.r = vk::ComponentSwizzle::eIdentity;
			createInfo.components.g = vk::ComponentSwizzle::eIdentity;
			createInfo.components.b = vk::ComponentSwizzle::eIdentity;
			createInfo.components.a = vk::ComponentSwizzle::eIdentity;

			createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			m_SwapChainImageViews[i] = m_Device.device().createImageView(createInfo);
		}
	}

	void SdeSwapChain::createRenderPass()
	{
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.format = m_SwapChainImageFormat;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass = {};
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		vk::SubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		m_RenderPass = m_Device.device().createRenderPass(renderPassInfo);
	}

	void SdeSwapChain::createFramebuffers()
	{
		m_Framebuffers.resize(m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			vk::ImageView attachments[] = {
				m_SwapChainImageViews[i]
			};

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapChainExtent.width;
			framebufferInfo.height = m_SwapChainExtent.height;
			framebufferInfo.layers = 1;

			m_Framebuffers[i] = m_Device.device().createFramebuffer(framebufferInfo);
		}
	}

	void SdeSwapChain::createSyncObjects()
	{
		m_ImageSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			m_ImageSemaphores[i] = m_Device.device().createSemaphore(vk::SemaphoreCreateInfo());
			m_RenderFinishedSemaphores[i] = m_Device.device().createSemaphore(vk::SemaphoreCreateInfo());
			m_InFlightFences[i] = m_Device.device().createFence(vk::FenceCreateInfo({ vk::FenceCreateFlagBits::eSignaled }));
		}
	}

	vk::SurfaceFormatKHR SdeSwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return availableFormat;
		}

		return availableFormats[0];
	}

	vk::PresentModeKHR SdeSwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
				std::cout << "Present mode: Mailbox\n";
				return availablePresentMode;
			}
		}

		std::cout << "Present mode: V-Sync\n";
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D SdeSwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = m_WindowExtent;
			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
}

