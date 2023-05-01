#pragma once

#include "sde_window.h"
#include "sde_device.h"
#include "sde_renderer.h"

namespace sde {

	class App {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		App();
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void run();

	private:
		SdeWindow m_SdeWindow{WIDTH, HEIGHT, "Application"};
		SdeDevice m_SdeDevice{m_SdeWindow};
		SdeRenderer m_SdeRenderer{ m_SdeWindow, m_SdeDevice };
	};

}