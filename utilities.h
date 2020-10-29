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


    static void Create( VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize,
                            VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags memPropFlags,
                            VkBuffer& buffer, VkDeviceMemory& bufferMemory )
    {
        // buffer info (doesn't include assigning memory)
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = static_cast<VkDeviceSize>( bufferSize );
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

    static void Copy( VkDevice device, VkQueue transferQueue, VkCommandPool commandPool, 
                        VkDeviceSize bufferSize, VkBuffer& srcBuffer, VkBuffer& dstBuffer )
    {
        // allocate for temporary command buffer
        VkCommandBufferAllocateInfo cmdBuffAllocInfo{};
        cmdBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBuffAllocInfo.commandPool = commandPool;
        cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBuffAllocInfo.commandBufferCount = 1;

        VkCommandBuffer tempCmdBuff;    // temp : temporary; this is just for transfer, and then destroy
        // create temporary command buffer
        if( vkAllocateCommandBuffers( device, &cmdBuffAllocInfo, &tempCmdBuff )
            != VK_SUCCESS )
        throw std::runtime_error( "Failed to create temporary command buffer! ");

        // --- temporary command buffer recording ---
        VkCommandBufferBeginInfo cmdBuffBeginInfo{};
        cmdBuffBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBuffBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;   // because this cmdBuff gonna be immediately destroy after submitting it
        cmdBuffBeginInfo.pInheritanceInfo = nullptr;    // optional
        // temporary command buffer begin recording
        if( vkBeginCommandBuffer( tempCmdBuff, &cmdBuffBeginInfo)
            != VK_SUCCESS )
        throw std::runtime_error( "Failed to beginning recording the temporary buffer!" );

        // configure the region of memory
        VkBufferCopy bufferCopyRegion{};
        bufferCopyRegion.dstOffset = 0;
        bufferCopyRegion.srcOffset = 0;
        bufferCopyRegion.size = bufferSize;
        // copy source buffer to destination buffer
        vkCmdCopyBuffer( tempCmdBuff, srcBuffer, dstBuffer, 1, &bufferCopyRegion );

        // temporary command buffer end recording
        if( vkEndCommandBuffer( tempCmdBuff )
            != VK_SUCCESS )
        throw std::runtime_error( "Failed to end recording temporary command buffer!" );
        // -----------------------------------------

        // submit info
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &tempCmdBuff;

        // submit temporary command buffer to transferQueue
        if ( vkQueueSubmit( transferQueue, 1, &submitInfo, VK_NULL_HANDLE )
            != VK_SUCCESS )
        throw std::runtime_error( "Failed to submitting command buffer to transfer queue!" );

        // wait to temporary command buffer being executed by transfer queue
        vkQueueWaitIdle( transferQueue );

        // freeing temporary command buffer
        vkFreeCommandBuffers( device, commandPool, 1, &tempCmdBuff );
    }

/*
 *  from udemy :
 *      Index Buffer and Staging Buffer
*/

}

