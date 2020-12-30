#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <set>
#include <vector>
#include <iostream>
#include <stdexcept>

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
	VkQueue			presentationQueue;
	VkSurfaceKHR	surface;

	struct{
		VkPhysicalDevice physicalDevice;
		VkDevice		 locigalDevice;
	}mainDevice;

	void				GetPhysicalDevice();

	void				CreateInstance();
	void				CreateLogicalDevice();
	void				CreateSurface();

	bool				CheckInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool				CheckDeviceExtensionSupport(VkPhysicalDevice device);
	bool				CheckDeviceSuitable(VkPhysicalDevice device);

	QueueFamilyIndices	GetQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails	GetSwapChainDetails(VkPhysicalDevice device);

};

