#include "app.h"

namespace sde {
	App::App()
	{
		std::vector<SdeModel::Vertex> triangleVertices = {
			{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
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

		SdeModel::Builder builder;
		builder.vertices = triangleVertices;

		m_TriangleModel = std::make_unique<SdeModel>(m_SdeDevice, builder);
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
				m_TriangleModel->draw(commandBuffer);

				m_SdeRenderer.endSwapChainRenderPass(commandBuffer);

				m_SdeRenderer.endFrame();
			}
		}

		m_SdeDevice.device().waitIdle();
	}
}


