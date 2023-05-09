#include "sde_buffer.h"

namespace sde {
	SdeBuffer::SdeBuffer(
		SdeDevice& device, 
		uint64_t size, 
		vk::Flags<vk::BufferUsageFlagBits> usageFlags,
		vk::Flags<vma::AllocationCreateFlagBits> allocationFlags,
		vma::MemoryUsage memoryUsageFlags) : m_Device(device), m_AllocationFlags(allocationFlags), m_Size(size)
	{
		// Allocate new memory
		vk::BufferCreateInfo bufferInfo(vk::BufferCreateFlags(), size, usageFlags);

		vma::AllocationCreateInfo allocationCreateInfo(vma::AllocationCreateFlags(), memoryUsageFlags);
		allocationCreateInfo.flags = m_AllocationFlags;

		auto data = m_Device.getAllocator().createBuffer(bufferInfo, allocationCreateInfo, &m_AllocationInfo);
		m_Buffer = data.first;
		m_Allocation = data.second;
	}

	SdeBuffer::~SdeBuffer()
	{
		m_Device.getAllocator().destroyBuffer(m_Buffer, m_Allocation);
	}

	vk::Result SdeBuffer::map()
	{
		if (m_AllocationFlags & vma::AllocationCreateFlagBits::eMapped)
			return vk::Result::eSuccess;
		return m_Device.getAllocator().mapMemory(m_Allocation, &m_MappedData);
	}

	void SdeBuffer::unmap()
	{
		m_Device.getAllocator().unmapMemory(m_Allocation);
	}

	void SdeBuffer::writeTo(void* data)
	{
		if (m_AllocationFlags & vma::AllocationCreateFlagBits::eMapped) {
			memcpy(m_AllocationInfo.pMappedData, data, m_Size);
		}
		else {
			memcpy(m_MappedData, data, sizeof(data));
		}
	}
}