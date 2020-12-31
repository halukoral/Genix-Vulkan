#pragma once

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

// Indices (locations) of Queue families (if they exist at all)
struct QueueFamilyIndices  
{
	// Location of Graphics queue family
	int iGraphicsFamily = -1;

	// Location of Presentation queue family
	int iPresentationFamily = -1;

	bool isValid()
	{
		return iGraphicsFamily >= 0 && iPresentationFamily >= 0;
	}
};

struct SwapChainDetails
{
	// Surface properties (image size ex.)
	VkSurfaceCapabilitiesKHR		surfaceCapabilities;
	// Surface image formats (RGBA ex.)
	std::vector<VkSurfaceFormatKHR> formats;
	// How images should be presented to screen
	std::vector<VkPresentModeKHR>	presentationModes;

};

struct SwapChainImage
{
	VkImage			image;
	VkImageView		imageView;
};