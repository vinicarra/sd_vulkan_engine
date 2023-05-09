#pragma once

#include "sde_device.h"

#include <vulkan/vulkan.hpp>
#include <vector>
#include <unordered_map>

namespace sde {

	class SdeDescriptorSetLayout {
	public:

		using DescriptorSetLayoutBindingMap = std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>;

		class Builder {
		public:
			Builder(SdeDevice& device) : m_Device(device) {}

			Builder& addBinding(
				uint32_t bindingId,
				vk::DescriptorType descriptorType,
				vk::ShaderStageFlags stageFlags,
				uint32_t count = 1
			);
			
			std::unique_ptr<SdeDescriptorSetLayout> build() const;

		private:
			SdeDevice& m_Device;
			DescriptorSetLayoutBindingMap m_Bindings;
		};

		SdeDescriptorSetLayout(SdeDevice& device, DescriptorSetLayoutBindingMap bindings);

		SdeDescriptorSetLayout(const SdeDescriptorSetLayout&) = delete;
		SdeDescriptorSetLayout& operator=(const SdeDescriptorSetLayout&) = delete;

		vk::DescriptorSetLayout getDescriptorSetLayout() { return m_DescriptorSetLayout.get(); }

	private:
		SdeDevice& m_Device;
		vk::UniqueDescriptorSetLayout m_DescriptorSetLayout;
		DescriptorSetLayoutBindingMap m_Bindings;

		friend class SdeDescriptorWriter;
	};

	class SdeDescriptorPool {
	public:
		class Builder {
		public:
			Builder(SdeDevice& device) : m_Device(device) {}

			Builder& addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
			Builder& setPoolFlags(vk::Flags<vk::DescriptorPoolCreateFlagBits> flags);
			Builder& setMaxSets(uint32_t count);

			std::unique_ptr<SdeDescriptorPool> build() const;
		private:
			SdeDevice& m_Device;
			std::vector<vk::DescriptorPoolSize> m_PoolSizes = {};
			vk::Flags<vk::DescriptorPoolCreateFlagBits> m_PoolFlags = {};
			uint32_t m_MaxSets = 1000;
		};

		// End Builder definitions

		SdeDescriptorPool(SdeDevice& device, uint32_t maxSets, vk::Flags<vk::DescriptorPoolCreateFlagBits> flags, const std::vector<vk::DescriptorPoolSize>& poolSizes);

		vk::DescriptorSet SdeDescriptorPool::allocateDescriptor(const vk::DescriptorSetLayout descriptorSetLayout);

		SdeDescriptorPool(const SdeDescriptorPool&) = delete;
		SdeDescriptorPool& operator=(const SdeDescriptorPool&) = delete;

	private:
		SdeDevice& m_Device;
		vk::UniqueDescriptorPool m_DescriptorPool;

		friend class SdeDescriptorWriter;
	};

	class SdeDescriptorWriter {
	public:
		SdeDescriptorWriter(SdeDescriptorSetLayout& setLayout, SdeDescriptorPool& descriptorPool);

		SdeDescriptorWriter& writeBuffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfo);
		SdeDescriptorWriter& writeImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo);

		vk::DescriptorSet build();
		void overwrite(vk::DescriptorSet& descriptorSet);

	private:
		SdeDescriptorSetLayout& m_DescriptorSetLayout;
		SdeDescriptorPool& m_DescriptorPool;
		std::vector<vk::WriteDescriptorSet> m_Writes;
	};

}