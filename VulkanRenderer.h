#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <set>
#include <array>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "VulkanWindow.h"
#include "Utilities.h"

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	VulkanWindow* GetVulkanWindow() { return window; }

private:
	VkInstance					instance;
	VulkanWindow*				window = nullptr;
	VkQueue						graphicsQueue;
	VkQueue						presentationQueue;
	VkSurfaceKHR				surface;
	VkSwapchainKHR				swapChain;

	// - Pipeline
	VkPipeline					graphicsPipeline;
	VkPipelineLayout			pipelineLayout;
	VkRenderPass				renderPass;

	std::vector<SwapChainImage> swapChainImages;

	struct{
		VkPhysicalDevice		physicalDevice;
		VkDevice				logicalDevice;
	}mainDevice;

	// Utility
	VkFormat					swapChainImageFormat;
	VkExtent2D					swapChainExtent;

	void				GetPhysicalDevice();

	void				CreateInstance();
	void				CreateLogicalDevice();
	void				CreateSurface();
	void				CreateSwapChain();
	void				CreateGraphicsPipeline();
	void				CreateRenderPass();

	bool				CheckInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool				CheckDeviceExtensionSupport(VkPhysicalDevice device);
	bool				CheckDeviceSuitable(VkPhysicalDevice device);

	QueueFamilyIndices	GetQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails	GetSwapChainDetails(VkPhysicalDevice device);

	VkSurfaceFormatKHR	ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR	ChooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes);
	VkExtent2D			ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	VkImageView			CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkShaderModule		CreateShaderModule(const std::vector<char>& code);

};

