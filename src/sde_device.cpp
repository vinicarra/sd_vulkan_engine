#include "sde_device.h"

namespace sde {

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	sde::SdeDevice::SdeDevice(SdeWindow& window) : m_SdeWindow(window)
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
		createAllocator();
	}

	sde::SdeDevice::~SdeDevice()
	{
		m_Allocator.destroy();
		m_Device.get().destroyCommandPool(m_CommandPool);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_Instance.get(), m_DebugMessenger, nullptr);
		}

		m_Instance.get().destroySurfaceKHR(m_Surface);
	}

	vk::CommandBuffer SdeDevice::beginSingleTimeCommand()
	{
		vk::CommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.level = vk::CommandBufferLevel::ePrimary;
		allocateInfo.commandPool = m_CommandPool;
		allocateInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer = m_Device->allocateCommandBuffers(allocateInfo)[0];
		
		commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		return commandBuffer;
	}

	void SdeDevice::endSingleTimeCommand(vk::CommandBuffer commandBuffer)
	{
		commandBuffer.end();

		// Submit
		vk::SubmitInfo submitInfo = {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		m_GraphicsQueue.submit(submitInfo);
		m_GraphicsQueue.waitIdle();

		m_Device->freeCommandBuffers(m_CommandPool, commandBuffer);;
	}

	void SdeDevice::copyBuffer(vk::Buffer src, vk::Buffer dst, uint64_t size)
	{
		auto commandBuffer = beginSingleTimeCommand();

		vk::BufferCopy copyRegion = {};
		copyRegion.size = size;

		commandBuffer.copyBuffer(src, dst, 1, &copyRegion);
		endSingleTimeCommand(commandBuffer);
	}

	void sde::SdeDevice::createInstance()
	{
		vk::ApplicationInfo appInfo(m_SdeWindow.getName().c_str(), 1, "No Engine", 1, VK_API_VERSION_1_1);

		auto glfwExtensions = getRequiredExtensions();

		vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo, 0, nullptr, static_cast<uint32_t>(glfwExtensions.size()), glfwExtensions.data());

		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}

		m_Instance = vk::createInstanceUnique(createInfo);
	}

	void SdeDevice::setupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		vk::DebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		std::cout << "Validation layers enabled\n";

		if (CreateDebugUtilsMessengerEXT(m_Instance.get(), reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, &m_DebugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("Failed create debug callback");
		}
	}

	void SdeDevice::createSurface()
	{
		m_Surface = m_SdeWindow.createSurface(m_Instance.get());
	}

	void sde::SdeDevice::pickPhysicalDevice()
	{
		auto physicalDevices = m_Instance.get().enumeratePhysicalDevices();

		if (physicalDevices.size() == 0)
			throw std::runtime_error("Failed to find a GPU");

		for (const auto& physicalDevice : physicalDevices)
		{
			if (isDeviceSuitable(physicalDevice))
			{
				m_PhysicalDevice = physicalDevice;
				break;
			}
		}

		if (!m_PhysicalDevice)
			throw std::runtime_error("Failed to find a suitable device");

		auto properties = m_PhysicalDevice.getProperties();

		std::cout << "Found GPU: " << properties.deviceName << " - v" << properties.driverVersion << std::endl;
	}

	void sde::SdeDevice::createLogicalDevice()
	{
		auto queueIndices = findQueueFamilies(m_PhysicalDevice);
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value() };

		float queuePriority = 0.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			queueCreateInfos.push_back({
				vk::DeviceQueueCreateFlags(),
				queueFamily,
				1,
				&queuePriority
			});
		}

		auto deviceFeatures = vk::PhysicalDeviceFeatures();
		auto createInfo = vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			static_cast<uint32_t>(queueCreateInfos.size()),
			queueCreateInfos.data()
		);

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(deviceExtensions.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		m_Device = m_PhysicalDevice.createDeviceUnique(createInfo);

		m_GraphicsQueue = m_Device.get().getQueue(queueIndices.graphicsFamily.value(), 0);
		m_PresentQueue = m_Device.get().getQueue(queueIndices.presentFamily.value(), 0);
	}

	void SdeDevice::createCommandPool()
	{
		auto queueIndices = findQueueFamilies(m_PhysicalDevice);

		vk::CommandPoolCreateInfo poolInfo = {};

		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = queueIndices.graphicsFamily.value();

		m_CommandPool = m_Device.get().createCommandPool(poolInfo);
	}

	void SdeDevice::createAllocator()
	{
		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
		allocatorCreateInfo.physicalDevice = m_PhysicalDevice;
		allocatorCreateInfo.device = m_Device.get();
		allocatorCreateInfo.instance = m_Instance.get();
		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

		m_Allocator = vma::createAllocator(allocatorCreateInfo);
	}

	std::vector<const char*> SdeDevice::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void SdeDevice::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		createInfo.pfnUserCallback = debugCallback;
	}

	bool SdeDevice::isDeviceSuitable(const vk::PhysicalDevice& device)
	{
		auto indices = findQueueFamilies(device);
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			auto swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	QueueFamilyIndices SdeDevice::findQueueFamilies(vk::PhysicalDevice physicalDevice)
	{
		QueueFamilyIndices indices;
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		int i = 0;
		for (const auto& queueFamily : queueFamilyProperties) {
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
				indices.graphicsFamily = i;
			if (queueFamily.queueCount > 0 && physicalDevice.getSurfaceSupportKHR(i, m_Surface))
				indices.presentFamily = i;
			if (indices.isComplete())
				break;
			i++;
		}

		return indices;
	}

	SwapChainSupportDetails SdeDevice::querySwapChainSupport(vk::PhysicalDevice physicalDevice)
	{
		SwapChainSupportDetails details;

		details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_Surface);
		details.formats = physicalDevice.getSurfaceFormatsKHR(m_Surface);
		details.presentModes = physicalDevice.getSurfacePresentModesKHR(m_Surface);

		return details;
	}


	bool SdeDevice::checkDeviceExtensionSupport(vk::PhysicalDevice physicalDevice)
	{
		auto extensions = physicalDevice.enumerateDeviceExtensionProperties();

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : extensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}
}


