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
	}
	catch (const std::runtime_error& e)
	{
		std::cout << "ERROR: " << e.what();
	}
}

VulkanRenderer::~VulkanRenderer()
{
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

bool VulkanRenderer::CheckInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
	// IMPORTANT
	// Obtaining list of objects is a fairly common operation in Vulkan, 
	// and the API has a consistent pattern for doing so. 

	// Call the function with a valid pointer to an integer for the count argumentand a NULL for the pointer argument.
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
	// Info about the device itself (ID, name, type, vendor, etc)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Info about what the device can do (geo shader, tess shader, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

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









