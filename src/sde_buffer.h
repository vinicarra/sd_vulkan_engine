#pragma once

#include "sde_device.h"
#include "vk_mem_alloc.hpp"

#include <vulkan/vulkan.hpp>

namespace sde {
	class SdeBuffer {
	public:
		SdeBuffer(
			SdeDevice& device,
			uint64_t size, 
			vk::Flags<vk::BufferUsageFlagBits> usageFlags,
			vk::Flags<vma::AllocationCreateFlagBits> allocationFlags = {},
			vma::MemoryUsage memoryUsageFlags = vma::MemoryUsage::eAuto
		);
		~SdeBuffer();

		SdeBuffer(const SdeBuffer&) = delete;
		SdeBuffer& operator=(const SdeBuffer&) = delete;

	public:
		vk::Buffer getBuffer() { return m_Buffer; }
		vma::Allocation getAllocation() { return m_Allocation; }
		vma::AllocationInfo getAllocationInfo() { return m_AllocationInfo; }

		vk::Result map();
		void unmap();
		void writeTo(void* data);

	private:
		SdeDevice& m_Device;
		vma::Allocation m_Allocation;
		vk::Buffer m_Buffer;
		vma::AllocationInfo m_AllocationInfo;

		uint64_t m_Size;
		void* m_MappedData = nullptr;
		
		vk::Flags<vma::AllocationCreateFlagBits> m_AllocationFlags;
	};
}