#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>

#include "VulkanWindow.h"
#include "Utilities.h"

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	VulkanWindow* GetVulkanWindow() { return window; }

private:
	VkInstance		instance;
	VulkanWindow*	window = nullptr;
	VkQueue			graphicsQueue;

	struct{
		VkPhysicalDevice physicalDevice;
		VkDevice		 locigalDevice;
	}mainDevice;

	void				GetPhysicalDevice();

	void				CreateInstance();
	void				CreateLogicalDevice();

	bool				CheckInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool				CheckDeviceSuitable(VkPhysicalDevice device);

	QueueFamilyIndices	GetQueueFamilies(VkPhysicalDevice device);

};

