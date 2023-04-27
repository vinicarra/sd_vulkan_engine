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
		}
	}
}


