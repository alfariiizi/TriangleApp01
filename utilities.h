#pragma once

#include <vulkan/vulkan.h>
#include <glm/vec3.hpp>

#include <optional>
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

struct QueueFamilyIndices
{
public:
    const bool IsComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
public:
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;  // presentation family
};

struct Vertex
{
    glm::vec3 pos;
    
/*
 *  from udemy :
 *      vertex input
*/
};