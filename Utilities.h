#pragma once

#include <fstream>

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

static std::vector<char> readFile(const std::string& filename)
{
	// Open stream from given file
	// std::ios::binary tells stream to read file as binary
	// std::ios::ate tells stream to start reading from end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	// Check if file stream successfully opened
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open a file!");
	}

	// Get current read position and use to resize file buffer
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> fileBuffer(fileSize);

	// Move read position (seek to) the start of the file
	file.seekg(0);

	// Read the file data into the buffer (stream "fileSize" in total)
	file.read(fileBuffer.data(), fileSize);

	// Close stream
	file.close();

	return fileBuffer;
}












