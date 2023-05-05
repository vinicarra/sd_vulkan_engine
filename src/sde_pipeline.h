#pragma once

#include "sde_device.h"
#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>

namespace sde {

	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {};
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {};
		vk::PipelineViewportStateCreateInfo viewportInfo;
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
		vk::PipelineMultisampleStateCreateInfo multisampleInfo;
		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
		vk::PipelineLayout pipelineLayout = nullptr;
		vk::RenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class SdePipeline {
	public:
		SdePipeline(SdeDevice& device, const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo);
		~SdePipeline();

		SdePipeline(const SdePipeline&) = delete;
		SdePipeline& operator=(const SdePipeline&) = delete;

		void bind(vk::CommandBuffer commandBuffer);
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);

	private:
		void createGraphicsPipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo);
		vk::UniqueShaderModule createShaderModule(const std::vector<char>& shaderCode);

		static std::vector<char> readFile(const std::string& path);

	private:
		SdeDevice& m_Device;
		vk::Pipeline m_Pipeline;
		vk::UniqueShaderModule m_VertexShaderModule, m_FragmentShaderModule;
	};
}