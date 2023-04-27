#include "sde_window.h"

namespace sde {
	SdeWindow::SdeWindow(int w, int h, std::string name) : m_Width(w), m_Height(h), m_Name(name)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(m_Width, m_Height, name.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
	}

	SdeWindow::~SdeWindow()
	{
		glfwDestroyWindow(m_Window);
	}

	vk::SurfaceKHR sde::SdeWindow::createSurface(vk::Instance instance)
	{
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
		return surface;
	}

	void SdeWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto sdeWindow = reinterpret_cast<SdeWindow*>(glfwGetWindowUserPointer(window));
		sdeWindow->m_HasResized = true;
		sdeWindow->m_Width = width;
		sdeWindow->m_Height = height;
	}
}


