#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct SwapchainSupportDetails
{
public:
    bool IsEmpty() const
    {
        return formats.empty() && presentModes.empty();
    }

public:
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/*
 *  from :
 *      Swapchain -> Querying details of swapchain support 
*/