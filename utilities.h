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
    glm::vec3 pos;  // position
    glm::vec3 col;  // color
    
/*
 *  from udemy :
 *      vertex input
*/
};

namespace Buffer
{
    static int32_t FindProperties( const VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                            uint32_t memoryTypeBitsRequirement, 
                            VkMemoryPropertyFlags requiredPropertiesFlags )
    {
        const uint32_t memoryCount = pMemoryProperties->memoryTypeCount;
        for( uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex )
        {
            const uint32_t memoryTypeBits = (1 << memoryIndex);
            const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

            const VkMemoryPropertyFlags properties = pMemoryProperties->memoryTypes[memoryIndex].propertyFlags;
            const bool hasRequiredPropertiesFlags = (properties & requiredPropertiesFlags) == requiredPropertiesFlags;

            if( isRequiredMemoryType && hasRequiredPropertiesFlags )
            {
                return static_cast<int32_t>(memoryIndex);
            }
        }

        return -1;  // failed to find memory type
    }


    static void CreateBuffer( VkPhysicalDevice physicalDevice, VkDevice device, size_t bufferSize,
                            VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags memPropFlags,
                            VkBuffer& buffer, VkDeviceMemory& bufferMemory )
    {
        // buffer info (doesn't include assigning memory)
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = static_cast<uint64_t>( bufferSize );
        bufferInfo.usage = bufferUsage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;       // optional: just for concurent sharing mode
        bufferInfo.pQueueFamilyIndices = nullptr;   // optional: just for concurent sharing mode
        if( vkCreateBuffer( device, &bufferInfo, nullptr, &buffer)
            != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to create vertex buffer!" );
        }

        // get memory requirements from vertexBuffer
        VkMemoryRequirements memReq{};
        vkGetBufferMemoryRequirements( device, buffer, &memReq );

        // memory allocate info
        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = memReq.size;

        VkPhysicalDeviceMemoryProperties memProps{};
        vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProps );
        int32_t memoryType = FindProperties( &memProps, 
                                            memReq.memoryTypeBits,
                                            memPropFlags );
        
        allocateInfo.memoryTypeIndex = memoryType;
        if( vkAllocateMemory( device, &allocateInfo, nullptr, &bufferMemory)
            != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to allocate memory for vertex buffer!" );
        }

        // binding vertex buffer with the memory allocation
        vkBindBufferMemory( device, buffer, bufferMemory, 0 );
    }

/*
 *  from udemy :
 *      Index Buffer and Staging Buffer
*/

}

