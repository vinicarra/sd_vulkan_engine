#pragma once

#include "sde_device.h"
#include "sde_window.h"
#include "sde_swap_chain.h"
#include <vulkan/vulkan.hpp>

namespace sde {

	class SdeRenderer {
	public:
		SdeRenderer(SdeWindow& window, SdeDevice& device);
		~SdeRenderer();

		SdeRenderer(const SdeRenderer&) = delete;
		SdeRenderer& operator =(const SdeRenderer&) = delete;

	public:
		vk::CommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(vk::CommandBuffer buffer);
		void endSwapChainRenderPass(vk::CommandBuffer buffer);

		vk::CommandBuffer getCurrentCommandBuffer() const {
			return m_CommandBuffers[m_CurrentFrameIndex];
		}

	private:
		void recreateSwapChain();
		void createCommandBuffers();
		void freeCommandBuffers();

	private:
		SdeWindow& m_SdeWindow;
		SdeDevice& m_SdeDevice;
		std::shared_ptr<SdeSwapChain> m_SdeSwapChain;

		std::vector<vk::CommandBuffer> m_CommandBuffers;

		uint32_t m_CurrentImageIndex;
		int m_CurrentFrameIndex = 0;
	};

}