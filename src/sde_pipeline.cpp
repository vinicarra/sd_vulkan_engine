#include "sde_pipeline.h"

#include <fstream>
#include <iostream>

namespace sde {
	SdePipeline::SdePipeline(
		SdeDevice& device, 
		const std::string& vertexPath, 
		const std::string& fragmentPath, 
		const PipelineConfigInfo& configInfo) : m_Device(device)
	{
		createGraphicsPipeline(vertexPath, fragmentPath, configInfo);
	}

	SdePipeline::~SdePipeline()
	{
		m_Device.device().destroyPipeline(m_Pipeline);
	}

	void SdePipeline::bind(vk::CommandBuffer commandBuffer)
	{
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
	}

	void SdePipeline::createGraphicsPipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo)
	{
		auto vertexCode = readFile(vertexPath);
		auto fragmentCode = readFile(fragmentPath);

		m_VertexShaderModule = createShaderModule(vertexCode);
		m_FragmentShaderModule = createShaderModule(fragmentCode);

		vk::PipelineShaderStageCreateInfo shaderStages[] = {
			{
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eVertex,
				m_VertexShaderModule.get(),
				"main"
			},
			{
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eFragment,
				m_FragmentShaderModule.get(),
				"main"
			}
		};

		auto& bindingDescription = configInfo.bindingDescriptions;
		auto& attributeDescriptions = configInfo.attributeDescriptions;

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// Create pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &configInfo.viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDynamicState = nullptr;

		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		try {
			auto pipilineVkResult = m_Device.device().createGraphicsPipeline(nullptr, pipelineInfo);
			if (pipilineVkResult.result != vk::Result::eSuccess)
				throw new std::runtime_error("Failed to create graphics pipeline");

			m_Pipeline = pipilineVkResult.value;
		}
		catch (vk::SystemError err) {
			throw new std::runtime_error("Failed to create graphics pipeline");
		}
	}

	vk::UniqueShaderModule SdePipeline::createShaderModule(const std::vector<char>& shaderCode)
	{
		vk::ShaderModuleCreateInfo createInfo = {};
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		return m_Device.device().createShaderModuleUnique(createInfo);
	}

	void SdePipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
	{
		configInfo.inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		configInfo.viewportInfo.viewportCount = 1;
		configInfo.viewportInfo.pViewports = nullptr;
		configInfo.viewportInfo.scissorCount = 1;
		configInfo.viewportInfo.pScissors = nullptr;

		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
		configInfo.rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
		configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
		configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		configInfo.colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;

		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = vk::LogicOp::eCopy;
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		configInfo.bindingDescriptions = SdeModel::Vertex::getBindingDescriptions();
		configInfo.attributeDescriptions = SdeModel::Vertex::getAttributeDescriptions();
	}

	void SdePipeline::enableAlphaBlending(PipelineConfigInfo& configInfo)
	{
		configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
		configInfo.colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		configInfo.colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		configInfo.colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		configInfo.colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		configInfo.colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
	}

	std::vector<char> SdePipeline::readFile(const std::string& path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw new std::runtime_error("Failed to open shader file: " + path);

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}
}