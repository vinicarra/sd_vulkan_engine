#include "app.h"

namespace sde {
	App::App()
	{
	}

	App::~App()
	{
	}

	void App::run()
	{
		while (!m_SdeWindow.shouldClose()) {
			glfwPollEvents();
			if (auto commandBuffer = m_SdeRenderer.beginFrame()) {

				m_SdeRenderer.beginSwapChainRenderPass(commandBuffer);
				m_SdeRenderer.endSwapChainRenderPass(commandBuffer);

				m_SdeRenderer.endFrame();
			}
		}

		m_SdeDevice.device().waitIdle();
	}
}


