#pragma once

#include "sde_device.h"

#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>
#include <memory>

namespace sde {

	class SdeSwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		SdeSwapChain(SdeDevice& device, vk::Extent2D windowExtent);
		SdeSwapChain(SdeDevice& device, vk::Extent2D windowExtent, std::shared_ptr<SdeSwapChain> oldSwapChain);

		~SdeSwapChain();

		SdeSwapChain(const SdeSwapChain&) = delete;
		SdeSwapChain& operator=(const SdeSwapChain&) = delete;

		vk::Framebuffer getFramebuffer(int index) { return m_Framebuffers[index]; }
		vk::RenderPass getRenderPass() { return m_RenderPass; }
		vk::ImageView getImageView(int index) { return m_SwapChainImageViews[index]; }
		vk::Format getSwapChainImageFormat() { return m_SwapChainImageFormat; }
		vk::Extent2D getSwapChainExtent() { return m_SwapChainExtent; }

		uint32_t getWidth() { return m_SwapChainExtent.width; }
		uint32_t getHeight() { return m_SwapChainExtent.height; }
		float getAspectRatio() { return (float)getWidth() / (float)getHeight(); }

		vk::ResultValue<uint32_t> acquireNextImage();
		vk::Result submitCommandBuffers(const vk::CommandBuffer* buffers, uint32_t imageIndex);

		bool compareSwapFormats(const SdeSwapChain& swapChain) const {
			return swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat;
		}

	private:
		void init();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		// Helper methods
		vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
		vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
		vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

	private:
		uint32_t m_CurrentFrame = 0;

		SdeDevice& m_Device;
		vk::Extent2D m_WindowExtent;
		vk::RenderPass m_RenderPass;

		vk::SwapchainKHR m_SwapChain;
		std::shared_ptr<SdeSwapChain> m_OldSwapChain;

		vk::Extent2D m_SwapChainExtent;
		std::vector<vk::Image> m_SwapChainImages;
		std::vector<vk::ImageView> m_SwapChainImageViews;

		vk::Format m_SwapChainImageFormat;

		// Framebuffers
		std::vector<vk::Framebuffer> m_Framebuffers;

		// Fences & Semaphores
		std::vector<vk::Semaphore> m_ImageSemaphores;
		std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
		std::vector<vk::Fence> m_InFlightFences;
	};

}