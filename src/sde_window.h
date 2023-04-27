#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan.hpp>

namespace sde {

	class SdeWindow {
	public:
		SdeWindow(int w, int h, std::string name);
		~SdeWindow();

		SdeWindow(const SdeWindow&) = delete;
		SdeWindow& operator=(const SdeWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(m_Window); }
		vk::Extent2D getExtent() { return { static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height) }; }
		bool hasResized() { return m_HasResized; }
		void resetResized() { m_HasResized = false; }
		GLFWwindow* getWindow() { return m_Window; };
		std::string getName() { return m_Name; }

		vk::SurfaceKHR createSurface(vk::Instance instance);

	private:
		GLFWwindow* m_Window;
		int m_Width, m_Height;
		std::string& m_Name;

		bool m_HasResized = false;

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	};


}