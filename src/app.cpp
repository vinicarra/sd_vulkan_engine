#include "app.h"

namespace sde {
	App::App()
	{
		std::vector<SdeModel::Vertex> triangleVertices = {
			{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
		};

		std::vector<SdeModel::Vertex> rectangleVertices = {
			{{  0.5f,  0.5f, 0.0f }, {1.0f, 0.0f, 0.0f}}, // bottom right
			{{ -0.5f,  0.5f, 0.0f }, {0.0f, 1.0f, 0.0f}}, // bottom left
			{{  0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 1.0f}}, // top right
			{{ -0.5f, -0.5f, 0.0f }, {0.5f, 0.25f, 0.8f}}  // top left
		};

		std::vector<uint32_t> rectangleIndices = {
			1, 2, 3,
			0, 1, 2
		};

		PipelineConfigInfo configInfo;
		SdePipeline::defaultPipelineConfigInfo(configInfo);

		configInfo.renderPass = m_SdeRenderer.getSwapChainRenderPass();

		// TODO: Remove pipeline layout from here
		m_PipelineLayout = m_SdeDevice.device().createPipelineLayout(vk::PipelineLayoutCreateInfo());

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

	void App::run()
	{
		while (!m_SdeWindow.shouldClose()) {
			glfwPollEvents();
			if (auto commandBuffer = m_SdeRenderer.beginFrame()) {

				m_SdeRenderer.beginSwapChainRenderPass(commandBuffer);

				// Render
				m_DefaultPipeline->bind(commandBuffer);

				m_TriangleModel->bind(commandBuffer);
				//m_TriangleModel->draw(commandBuffer);

				m_RectangleModel->bind(commandBuffer);
				m_RectangleModel->draw(commandBuffer);

				m_SdeRenderer.endSwapChainRenderPass(commandBuffer);

				m_SdeRenderer.endFrame();
			}
		}

		m_SdeDevice.device().waitIdle();
	}
}


