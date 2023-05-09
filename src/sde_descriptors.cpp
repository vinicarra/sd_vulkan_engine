#include "sde_descriptors.h"

namespace sde {

	// Descriptor Pool Builder

	SdeDescriptorPool::Builder& SdeDescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType, uint32_t count)
	{
		m_PoolSizes.push_back({ descriptorType, count });
		return *this;
	}

	SdeDescriptorPool::Builder& SdeDescriptorPool::Builder::setPoolFlags(vk::Flags<vk::DescriptorPoolCreateFlagBits> flags)
	{
		m_PoolFlags = flags;
		return *this;
	}

	SdeDescriptorPool::Builder& SdeDescriptorPool::Builder::setMaxSets(uint32_t count)
	{
		m_MaxSets = count;
		return *this;
	}

	std::unique_ptr<SdeDescriptorPool> SdeDescriptorPool::Builder::build() const
	{
		return std::make_unique<SdeDescriptorPool>(m_Device, m_MaxSets, m_PoolFlags, m_PoolSizes);
	}

	// Descriptor Pool

	SdeDescriptorPool::SdeDescriptorPool(SdeDevice& device, uint32_t maxSets, 
		vk::Flags<vk::DescriptorPoolCreateFlagBits> flags, const std::vector<vk::DescriptorPoolSize>& poolSizes) : m_Device(device)
	{
		vk::DescriptorPoolCreateInfo createInfo = {};
		createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		createInfo.pPoolSizes = poolSizes.data();
		createInfo.maxSets = maxSets;
		createInfo.flags = flags;

		m_DescriptorPool = m_Device.device().createDescriptorPoolUnique(createInfo);
	}

	vk::DescriptorSet SdeDescriptorPool::allocateDescriptor(const vk::DescriptorSetLayout descriptorSetLayout)
	{
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.descriptorPool = m_DescriptorPool.get();
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		try {
			return m_Device.device().allocateDescriptorSets(allocInfo).at(0);
		}
		catch (vk::SystemError err) {
			throw std::runtime_error("Failed to allocated descriptor set");
		}

	}

	// Descriptor Set Layout Builder

	SdeDescriptorSetLayout::Builder& SdeDescriptorSetLayout::Builder::addBinding(uint32_t bindingId, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32_t count)
	{
		vk::DescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = bindingId;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;

		m_Bindings[bindingId] = layoutBinding;
		return *this;
	}

	std::unique_ptr<SdeDescriptorSetLayout> SdeDescriptorSetLayout::Builder::build() const
	{
		return std::make_unique<SdeDescriptorSetLayout>(m_Device, m_Bindings);
	}

	// Descriptor Set Layout

	SdeDescriptorSetLayout::SdeDescriptorSetLayout(SdeDevice& device, DescriptorSetLayoutBindingMap bindings) : m_Device(device), m_Bindings(bindings)
	{
		// 1. Convert map to array
		std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings = {};
		for (auto& [id, set] : m_Bindings) {
			setLayoutBindings.push_back(set);
		}

		// 2. Create descriptor set
		vk::DescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		createInfo.pBindings = setLayoutBindings.data();

		m_DescriptorSetLayout = m_Device.device().createDescriptorSetLayoutUnique(createInfo);
	}

	// Descriptor writer

	SdeDescriptorWriter::SdeDescriptorWriter(SdeDescriptorSetLayout& setLayout, SdeDescriptorPool& descriptorPool) : m_DescriptorSetLayout(setLayout), m_DescriptorPool(descriptorPool)
	{
	}

	SdeDescriptorWriter& SdeDescriptorWriter::writeBuffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfo)
	{
		auto& bindingDescription = m_DescriptorSetLayout.m_Bindings.at(binding);

		vk::WriteDescriptorSet write = {};
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;

		m_Writes.push_back(write);
		return *this;
	}

	SdeDescriptorWriter& SdeDescriptorWriter::writeImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo)
	{
		auto& bindingDescription = m_DescriptorSetLayout.m_Bindings.at(binding);

		vk::WriteDescriptorSet write = {};
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;

		m_Writes.push_back(write);
		return *this;
	}

	vk::DescriptorSet SdeDescriptorWriter::build()
	{
		auto set = m_DescriptorPool.allocateDescriptor(m_DescriptorSetLayout.getDescriptorSetLayout());
		overwrite(set);
		return set;
	}

	void SdeDescriptorWriter::overwrite(vk::DescriptorSet& descriptorSet)
	{
		for (auto& write : m_Writes) {
			write.dstSet = descriptorSet;
		}
		m_DescriptorPool.m_Device.device().updateDescriptorSets(m_Writes, nullptr);
	}

}


