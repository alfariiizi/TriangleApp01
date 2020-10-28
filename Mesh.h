#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <memory.h>

#include "utilities.h"

class Mesh
{
public:
    Mesh() = default;
    Mesh( VkPhysicalDevice physicalDevice, VkDevice device, std::vector<Vertex>& vertices )
        :
        _physicalDevice( physicalDevice ),
        _device( device )
    {
        _vertexCount = (int)vertices.size();
        CreateVertexBuffer( vertices );
    }
    int GetVertexCount()    // for cmd buffer record
    {
        return _vertexCount;
    }
    VkBuffer GetVertexBuffer()
    {
        return _vertexBuffer;
    }
    void DestroyMeshesContent()
    {
        // destroy vertex buffer
        vkDestroyBuffer( _device, _vertexBuffer, nullptr );

        // freeing vertex buffer memory
        vkFreeMemory( _device, _vertexBufferMemory, nullptr );
    }

private:
    void CreateVertexBuffer( std::vector<Vertex>& vertices )
    {
        // buffer info (doesn't include assigning memory)
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = static_cast<uint64_t>( sizeof(Vertex) * vertices.size() );
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;       // optional: just for concurent sharing mode
        bufferInfo.pQueueFamilyIndices = nullptr;   // optional: just for concurent sharing mode
        if( vkCreateBuffer( _device, &bufferInfo, nullptr, &_vertexBuffer)
            != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to create vertex buffer!" );
        }

        // get memory requirements from vertexBuffer
        VkMemoryRequirements memReq{};
        vkGetBufferMemoryRequirements( _device, _vertexBuffer, &memReq );

        // memory allocate info
        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = memReq.size;

        VkPhysicalDeviceMemoryProperties memProps{};
        vkGetPhysicalDeviceMemoryProperties( _physicalDevice, &memProps );
        int32_t memoryType = FindProperties( &memProps, 
                                            memReq.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
        
        allocateInfo.memoryTypeIndex = memoryType;
        if( vkAllocateMemory( _device, &allocateInfo, nullptr, &_vertexBufferMemory)
            != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to allocate memory for vertex buffer!" );
        }

        // binding vertex buffer with the memory allocation
        vkBindBufferMemory( _device, _vertexBuffer, _vertexBufferMemory, 0 );

        // mapping the memory (yang baru saja dialokasikan) to vertex buffer
        void * data;
        vkMapMemory( _device, _vertexBufferMemory, 0, bufferInfo.size, 0, &data );
        memcpy( data, vertices.data(), size_t(bufferInfo.size) );
        vkUnmapMemory( _device, _vertexBufferMemory );
    }
    int32_t FindProperties( const VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                            uint32_t memoryTypeBitsRequirement, 
                            VkMemoryPropertyFlags requiredProperties )
    {
        const uint32_t memoryCount = pMemoryProperties->memoryTypeCount;
        for( uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex )
        {
            const uint32_t memoryTypeBits = (1 << memoryIndex);
            const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

            const VkMemoryPropertyFlags properties = pMemoryProperties->memoryTypes[memoryIndex].propertyFlags;
            const bool hasRequiredProperties = (properties & requiredProperties) == requiredProperties;

            if( isRequiredMemoryType && hasRequiredProperties )
            {
                return static_cast<int32_t>(memoryIndex);
            }
        }

        return -1;  // failed to find memory type
    }

private:
    int _vertexCount;
    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;

    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
};