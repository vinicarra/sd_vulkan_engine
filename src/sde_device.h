#pragma once

#include "vk_mem_alloc.hpp"
#include "sde_window.h"

#include <vector>
#include <string>
#include <set>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <iostream>

namespace sde {

	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};


	class SdeDevice {
	public:
	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif

		SdeDevice(SdeWindow& window);
		~SdeDevice();

		// Not copyable or movable
		SdeDevice(const SdeDevice&) = delete;
		SdeDevice& operator=(const SdeDevice&) = delete;
		SdeDevice(SdeDevice&&) = delete;
		SdeDevice& operator=(SdeDevice&&) = delete;

	public:
		vk::Queue graphicsQueue() { return m_GraphicsQueue; }
		vk::Queue presentQueue() { return m_PresentQueue; }
		vk::Device device() { return m_Device.get(); }
		vk::SurfaceKHR surface() { return m_Surface; }
		vk::CommandPool commandPool() { return m_CommandPool; }
		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_PhysicalDevice); }
		SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(m_PhysicalDevice); }

	private:
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();
		void createAllocator();

		// Helper functions
		std::vector<const char*> getRequiredExtensions();
		void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);

		bool isDeviceSuitable(const vk::PhysicalDevice& physicalDevice);
		bool checkDeviceExtensionSupport(vk::PhysicalDevice physicalDevice);
		QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice);
		SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice physicalDevice);

	private:
		vk::UniqueInstance m_Instance;
		vk::PhysicalDevice m_PhysicalDevice;
		vk::UniqueDevice m_Device;
		vk::SurfaceKHR m_Surface;
		vk::Queue m_GraphicsQueue, m_PresentQueue;
		vk::CommandPool m_CommandPool;

		vma::Allocator m_Allocator;

		VkDebugUtilsMessengerEXT m_DebugMessenger;

		SdeWindow& m_SdeWindow;

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}