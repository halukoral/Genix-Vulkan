#pragma once

// Indices (locations) of Queue families (if they exist at all)
struct QueueFamilyIndices  
{
	// Location of Graphics queue family
	int iGraphicsFamily = -1;

	bool isValid()
	{
		return iGraphicsFamily >= 0;
	}
};