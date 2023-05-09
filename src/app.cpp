#include "app.h"

namespace sde {
	App::App()
	{
		// Global pool
		m_GlobalPool = SdeDescriptorPool::Builder(m_SdeDevice)
			.setMaxSets(SdeSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(vk::DescriptorType::eUniformBuffer, SdeSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		initUBO();

		std::vector<SdeModel::Vertex> triangleVertices = {
			{{0.0f, -0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
		};

		std::vector<SdeModel::Vertex> rectangleVertices = {
			{{  0.5f,  0.5f, -1.0f }, {1.0f, 0.0f, 0.0f}}, // bottom right
			{{ -0.5f,  0.5f,  1.0f }, {0.0f, 1.0f, 0.0f}}, // bottom left
			{{  0.5f, -0.5f, -1.0f }, {0.0f, 0.0f, 1.0f}}, // top right
			{{ -0.5f, -0.5f,  1.0f }, {0.5f, 0.25f, 0.8f}}  // top left
		};

		std::vector<uint32_t> rectangleIndices = {
			1, 2, 3,
			0, 1, 2
		};

		PipelineConfigInfo configInfo;
		SdePipeline::defaultPipelineConfigInfo(configInfo);
		configInfo.renderPass = m_SdeRenderer.getSwapChainRenderPass();
		configInfo.pipelineLayout = m_PipelineLayout;

		m_DefaultPipeline = std::make_shared<SdePipeline>(
			m_SdeDevice,
			"../shaders/shader.vert.spv",
			"../shaders/shader.frag.spv",
			configInfo
		);

		SdeModel::Builder triangleBuilder;
		triangleBuilder.vertices = triangleVertices;

		SdeModel::Builder rectangleBuilder;
		rectangleBuilder.vertices = rectangleVertices;
		rectangleBuilder.indices = rectangleIndices;

		m_TriangleModel = std::make_unique<SdeModel>(m_SdeDevice, triangleBuilder);
		m_RectangleModel = std::make_unique<SdeModel>(m_SdeDevice, rectangleBuilder);
	}

	App::~App()
	{
		m_SdeDevice.device().destroyPipelineLayout(m_PipelineLayout);
	}

	void App::initUBO()
	{
		m_UboBuffers.resize(SdeSwapChain::MAX_FRAMES_IN_FLIGHT);
		m_DescriptorSets.resize(SdeSwapChain::MAX_FRAMES_IN_FLIGHT);

		// 1. Allocate UBO buffers
		for (size_t i = 0; i < m_UboBuffers.size(); i++) {
			m_UboBuffers[i] = std::make_unique<SdeBuffer>(
				m_SdeDevice,
				sizeof(GlobalUbo),
				vk::BufferUsageFlagBits::eUniformBuffer,
				vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
			);
		}

		// 2. Create set layout
		m_DescriptorSetLayout = SdeDescriptorSetLayout::Builder(m_SdeDevice)
			.addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll)
			.build();

		// 3. Create descriptor sets
		for (size_t i = 0; i < m_DescriptorSets.size(); i++) {
			vk::DescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = m_UboBuffers[i]->getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(GlobalUbo);

			m_DescriptorSets[i] = SdeDescriptorWriter(*m_DescriptorSetLayout, *m_GlobalPool)
				.writeBuffer(0, &bufferInfo)
				.build();
		}

		// TODO: Remove pipeline layout from here
		// 
		// 4. Create pipeline layout
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = { m_DescriptorSetLayout->getDescriptorSetLayout() };
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

		// 4.1 Add push constants

		m_PipelineLayout = m_SdeDevice.device().createPipelineLayout(pipelineLayoutCreateInfo);
	}

	void App::run()
	{
		while (!m_SdeWindow.shouldClose()) {
			glfwPollEvents();
			if (auto commandBuffer = m_SdeRenderer.beginFrame()) {
				uint32_t frameIndex = m_SdeRenderer.getFrameIndex();

				m_SdeRenderer.beginSwapChainRenderPass(commandBuffer);

				// Render
				GlobalUbo ubo = {};
				ubo.projection = glm::perspective(glm::radians(45.0f), m_SdeRenderer.getAspectRatio(), 0.1f, 10.0f);
				ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				ubo.model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

				m_UboBuffers[frameIndex]->writeTo(&ubo);

				m_DefaultPipeline->bind(commandBuffer);

				commandBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					m_PipelineLayout,
					0,
					1,
					&m_DescriptorSets[frameIndex], 
					0, 
					0
				);

				m_RectangleModel->bind(commandBuffer);
				m_RectangleModel->draw(commandBuffer);

				m_SdeRenderer.endSwapChainRenderPass(commandBuffer);

				m_SdeRenderer.endFrame();
			}
		}

		m_SdeDevice.device().waitIdle();
	}
	
}


