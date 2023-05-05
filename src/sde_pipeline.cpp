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
	}

	void SdePipeline::createGraphicsPipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo)
	{
		auto vertexCode = readFile(vertexPath);
		auto fragmentCode = readFile(fragmentPath);

		m_VertexShaderModule = createShaderModule(vertexCode);
		m_FragmentShaderModule = createShaderModule(fragmentCode);
	}

	vk::UniqueShaderModule SdePipeline::createShaderModule(const std::vector<char>& shaderCode)
	{
		vk::ShaderModuleCreateInfo createInfo = {};
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		return m_Device.device().createShaderModuleUnique(createInfo);
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