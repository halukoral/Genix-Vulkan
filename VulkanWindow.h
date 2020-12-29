#pragma once
#include <GLFW/glfw3.h>
#include <string>

class VulkanWindow
{
public:
	VulkanWindow(std::string wName, const int width, const int height);
	~VulkanWindow();

	GLFWwindow* GetWindow() { return Window; }
private:
	int				iWidth = 1280;
	int				iHeight = 720;

	std::string		WindowName = " ";

	GLFWwindow*		Window = nullptr;
};

