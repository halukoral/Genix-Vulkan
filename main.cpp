
#include "VulkanRenderer.h"
#include "VulkanWindow.h"
#include <iostream>

int main()
{
	VulkanRenderer* vulkanRenderer = new VulkanRenderer();

	while (!glfwWindowShouldClose(vulkanRenderer->GetVulkanWindow()->GetWindow()))
	{
		glfwPollEvents();
	}

	return 0;
}



