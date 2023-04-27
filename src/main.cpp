#include <iostream>
#include <vulkan/vulkan.hpp>
#include "app.h"

int main() {
	try {
		sde::App app;
		app.run();
	}
	catch (vk::SystemError& err)
	{
		std::cout << "vk::SystemError: " << err.what() << std::endl;
	}
	catch (const std::runtime_error& err)
	{
		std::cout << "std::runtime_error " << err.what() << std::endl;
	}
	catch (const std::exception& err)
	{
		std::cout << "std::exception " << err.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown error\n";
	}

}