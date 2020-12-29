#include "VulkanWindow.h"

VulkanWindow::VulkanWindow(std::string wName, const int width, const int height)
	: WindowName(wName), iWidth(width), iHeight(height)
{
	glfwInit();

	// Set GLFW to NOT work with OPENGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	Window = glfwCreateWindow(iWidth, iHeight, WindowName.c_str(), nullptr, nullptr);
}

VulkanWindow::~VulkanWindow()
{
	glfwDestroyWindow(Window);

	glfwTerminate();
}
