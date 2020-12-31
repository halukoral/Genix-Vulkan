#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
	window = new VulkanWindow("Test", 1280, 720);

	try 
	{
		CreateInstance();
		CreateSurface();
		GetPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
	}
	catch (const std::runtime_error& e)
	{
		std::cout << "ERROR: " << e.what();
	}
}

VulkanRenderer::~VulkanRenderer()
{
	for (auto image : swapChainImages)
	{
		vkDestroyImageView(mainDevice.locigalDevice, image.imageView, nullptr);
	}

	vkDestroySwapchainKHR(mainDevice.locigalDevice, swapChain, nullptr);
	vkDestroyInstance(instance, nullptr);
	vkDestroyDevice(mainDevice.locigalDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	delete window;
}

void VulkanRenderer::GetPhysicalDevice()
{
	// Enumerate Physical devices the vkInstance can access
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	// This function returns only a list of handles for each physical device on the system.

	// If no device is available, then none support VULKAN!
	if (deviceCount == 0)
	{
		throw std::runtime_error("Cant find GPUs that support Vulkan Instance!");
	}

	// Get list of physical devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

	for (const auto& device : deviceList)
	{
		if (CheckDeviceSuitable(device))
		{
			mainDevice.physicalDevice = device;
			break;
		}
	}
}

void VulkanRenderer::CreateInstance()
{
	// Info about the app itself
	// Most data here does not affect program
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan App";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Genix";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;


	// Creation information for VkInstance
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	// Create list to hold instance extensions
	std::vector<const char*> instanceExtensions = std::vector<const char*>();

	// Setup extensions Instance will use
	// GLFW may require multiple extension
	uint32_t glfwExtensionCount = 0;

	const char** glfwExtensions;

	// Get GLFW extensions
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfwExtensionCount; ++i)
	{
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	if (!CheckInstanceExtensionSupport(&instanceExtensions))
	{
		throw std::runtime_error("VkInstance does not support required extensions!");
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	// TODO: Set up validation layers that instance will used
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;

	// Create Instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan Instance!");
	}

}

void VulkanRenderer::CreateLogicalDevice()
{
	// Get queue family indices for the chosen Physical device
	QueueFamilyIndices indices = GetQueueFamilies(mainDevice.physicalDevice);
	
	// Vector for queue creation info, and set for family indices
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndices = { indices.iGraphicsFamily, indices.iPresentationFamily };

	// Queue the logical device need to create and info to do so 
	for (int q : queueFamilyIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = q;
		queueCreateInfo.queueCount = 1;
		// Vulkan needs to know how to handle multiple queues, so decide priority (1 is highest)
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Info to create the logical device
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	// Number of queue create infos
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	// List of queue create info so device can create required
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	// Number of enabled logical device extensions
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	// List of enabled logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	
	// Physical device features the logical device will be using
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create the logical device for the given physical device
	if (vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.locigalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	// Queues are created at the same time as the device
	// So we want to handle queue.
	vkGetDeviceQueue(mainDevice.locigalDevice, indices.iGraphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(mainDevice.locigalDevice, indices.iPresentationFamily, 0, &presentationQueue);


}

void VulkanRenderer::CreateSurface()
{
	// Create Surface
	if (glfwCreateWindowSurface(instance, window->GetWindow(), nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a surface!");
	}
}

void VulkanRenderer::CreateSwapChain()
{
	SwapChainDetails swp = GetSwapChainDetails(mainDevice.physicalDevice);

	// 1. CHOOSE BEST SURFACE FORMAT
	VkSurfaceFormatKHR surfaceFormat = ChooseBestSurfaceFormat(swp.formats);
		
	// 2. CHOOSE BEST PRESENTATION MODE
	VkPresentModeKHR presentMode = ChooseBestPresentationMode(swp.presentationModes);
	
	// 3. CHOOSE SWAP CHAIN IMAGE RESOLUTION
	VkExtent2D extent = ChooseSwapExtent(swp.surfaceCapabilities);

	// How many images are in the swap chain?
	// Get 1 more than the min to allow triple buffering
	uint32_t imageCount = swp.surfaceCapabilities.minImageCount + 1;
	if (swp.surfaceCapabilities.maxImageCount > 0 && swp.surfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = swp.surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;	
	swapChainCreateInfo.surface = surface;											// Swapchain surface
	swapChainCreateInfo.imageFormat = surfaceFormat.format;							// Swapchain format
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;					// Swapchain color space
	swapChainCreateInfo.presentMode = presentMode;									// Swapchain presentation modes
	swapChainCreateInfo.imageExtent = extent;										// Swapchain image extents
	swapChainCreateInfo.minImageCount = imageCount;									// Min images in swapchain
	swapChainCreateInfo.imageArrayLayers = 1;										// Num of layers for each image in chain
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;			// What attachment images will be used as
	swapChainCreateInfo.preTransform = swp.surfaceCapabilities.currentTransform;	// Transform to perform on swap chain images
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;			// How to handle blending images with external graphics (ex. other windows)
	swapChainCreateInfo.clipped = VK_TRUE;											// Whether to clip parts of image not in view (ex. behind another window, off screen etc.)
	
	// Get Queue Family indices
	QueueFamilyIndices indices = GetQueueFamilies(mainDevice.physicalDevice);
	// If graphics and presentation families are different, then swapchain must let images be shared between families
	if (indices.iGraphicsFamily != indices.iPresentationFamily)
	{
		// Queues to share between
		uint32_t queueFamilyIndices[] =
		{
			(uint32_t)indices.iGraphicsFamily,
			(uint32_t)indices.iPresentationFamily,
		};

		
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;			// Image share handling
		swapChainCreateInfo.queueFamilyIndexCount = 2;								// Num of queues to share image between
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;				// Array of queues to share between
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;							
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;			
	}

	// If old swap chain been destroyed and this one replaces it,
	// then link old one to quickly hand over responsibilities
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create Swap Chain
	if (vkCreateSwapchainKHR(mainDevice.locigalDevice, &swapChainCreateInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Swapchain!");
	}

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	// Get swap chain images
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mainDevice.locigalDevice, swapChain, &swapChainImageCount, nullptr);

	std::vector<VkImage> images(swapChainImageCount);
	vkGetSwapchainImagesKHR(mainDevice.locigalDevice, swapChain, &swapChainImageCount, images.data());

	for (VkImage image : images)
	{
		// Store image handle
		SwapChainImage swapChainImage = {};
		swapChainImage.image = image;

		// Create IMAGE VIEW here
		swapChainImage.imageView = CreateImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		
		// Add to swapchain image list
		swapChainImages.push_back(swapChainImage);
	}
}

bool VulkanRenderer::CheckInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
	// IMPORTANT
	// Obtaining list of objects is a fairly common operation in Vulkan, 
	// and the API has a consistent pattern for doing so. 

	// Call the function with a valid pointer to an integer for the count argument and a NULL for the pointer argument.
		// The API fills in the count argument with the number of objects in the list.
		// The application allocates enough space to store the list.
		// The application calls the function again with the pointer argument pointing to the space just allocated.
	// You will see this pattern often in the Vulkan API.

	// Need to get number of extensions to create array of correct size to hold extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	// Create list of vkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// Check if given extensions are in list of available extensions
	for (const auto& checkExtension : *checkExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(checkExtension, extension.extensionName))
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	// Get device extension count
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	if (extensionCount == 0)
		return false;

	// Create list of vkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

	// Check if given extensions are in list of available extensions
	for (const auto& deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName))
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

bool VulkanRenderer::CheckDeviceSuitable(VkPhysicalDevice device)
{
	/*
	// Info about the device itself (ID, name, type, vendor, etc)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Info about what the device can do (geo shader, tess shader, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	*/
	QueueFamilyIndices indices = GetQueueFamilies(device);

	bool bExtensionSupported = CheckDeviceExtensionSupport(device);

	bool bSwapChainValid = false;
	if (bExtensionSupported)
	{
		SwapChainDetails swp = GetSwapChainDetails(device);
		bSwapChainValid = !swp.presentationModes.empty() && !swp.formats.empty();
	}

	return indices.isValid() && bExtensionSupported && bSwapChainValid;
}

QueueFamilyIndices VulkanRenderer::GetQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// Get all queue family property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	// Go through each queue family and check if it has at least 1 of the required types of queue
	int i = 0;
	for (const auto& queueFamily : queueFamilyList)
	{
		// First check if queue family has at least 1 queue in that family
		// Queue can be multiple types defined through bitfield.
		// Need to bitwise AND with VK_QUEUE_*_BIT to check if has required type
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			// If queue family is valid then get index
			indices.iGraphicsFamily = i;
		}

		// Check if queue family supports presentation
		VkBool32 bPresentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &bPresentationSupport);
		if (queueFamily.queueCount > 0 && bPresentationSupport)
		{
			indices.iPresentationFamily = i;
		}

		// Check if queue family indices are in a valid state, stop searching if so
		if (indices.isValid())
		{
			break;
		}

		++i;
	}

	return indices;
}

SwapChainDetails VulkanRenderer::GetSwapChainDetails(VkPhysicalDevice device)
{
	SwapChainDetails swapChainDetails;

	// CAPABILITIES
	// Get the surface capabilities for the given surface on the given device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfaceCapabilities);

	// FORMATS
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
	}

	// PRESENTATION MODES
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);
	if (presentationCount != 0)
	{
		swapChainDetails.presentationModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainDetails.presentationModes.data());
	}
 
	return swapChainDetails;
}

VkSurfaceFormatKHR VulkanRenderer::ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	// this means ALL formats available
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	// If restricted search for optimal format
	for (const auto& format : formats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return formats[0];
}

VkPresentModeKHR VulkanRenderer::ChooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes)
{
	for (const auto& presentationMode : presentationModes)
	{
		if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentationMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window->GetWindow(), &width, &height);

		// Create new extent using window size
		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		// Surface also defines max and min, so make sure within boundaries by clamping value
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}

VkImageView VulkanRenderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;										// Image to create view for
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;					// Type of Image
	viewCreateInfo.format = format;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;		// Allow remapping of rgba components to other rgba values
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// Subresources allow the view to view only a part of an image
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;			// which aspect of image to view
	viewCreateInfo.subresourceRange.baseMipLevel = 0;					// Start mipmap level to view from
	viewCreateInfo.subresourceRange.levelCount = 1;						// Num of mipmap levels to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;					// Start aray level to view from
	viewCreateInfo.subresourceRange.layerCount = 1;						// Num of array levels to view
	
	// Create image and view and return it
	VkImageView imageView;
	if (vkCreateImageView(mainDevice.locigalDevice, &viewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		std::runtime_error("Failed to create image view!");
	}

	return imageView;
}





