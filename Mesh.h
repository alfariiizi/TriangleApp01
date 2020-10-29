#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <memory.h>

#include "utilities.h"

class Mesh
{
public:
    // bufferUsage
    enum struct UsageBuffer // it's the same as bufferUsage, but I swap the name in order to not make confusion with bufferUsage
    {
        VERTEX_BUFFER,
        INDEX_BUFFER
    };
private:
    struct Content
    {
        int count;
        VkBuffer buffer;
        VkDeviceMemory bufferMemory;
    };


public:
    Mesh() = default;
    Mesh( VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool cmdPool, UsageBuffer usage, std::vector<Vertex>& vertices )
        :
        _physicalDevice( physicalDevice ),
        _device( device )
    {
        _content.count = (int)vertices.size();
        CreateVertexBuffer( transferQueue, cmdPool, usage, vertices );
    }
    int GetCount()    // for cmd buffer record
    {
        return _content.count;
    }
    VkBuffer GetBuffer()
    {
        return _content.buffer;
    }

    void DestroyMeshesContent()
    {
        // destroy the buffer
        vkDestroyBuffer( _device, _content.buffer, nullptr );

        // freeing vertex buffer memory
        vkFreeMemory( _device, _content.bufferMemory, nullptr );
    }

private:
    void CreateVertexBuffer( VkQueue transferQueue, VkCommandPool cmdPool, UsageBuffer usage, std::vector<Vertex>& vertices )
    {
        VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
        

        // --- Staging Buffer (src buffer, store in CPU memory, CPU-GPU visible) ---
        VkBuffer staggingBuffer;
        VkDeviceMemory staggingBufferMemory;
        VkBufferUsageFlags transferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags staggingMemPropFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        Buffer::Create( _physicalDevice, _device, bufferSize, transferUsage, staggingMemPropFlags, staggingBuffer, staggingBufferMemory );

        // mapping the memory (yang baru saja dialokasikan) to vertex buffer
        void * data;
        vkMapMemory( _device, staggingBufferMemory, 0, bufferSize, 0, &data );
        memcpy( data, vertices.data(), size_t(bufferSize) );
        vkUnmapMemory( _device, staggingBufferMemory );
        // ----------------------

        // --- Vertex Buffer (dst buffer, store in GPU memory, GPU only visible) ---
        VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usagebufferlist[size_t(usage)];
        VkMemoryPropertyFlags memPropFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        
        Buffer::Create( _physicalDevice, _device, bufferSize, bufferUsage,
                               memPropFlags, _content.buffer, _content.bufferMemory );
        // ----------------------------------

        // copying stagging buffer to vertex buffer
        Buffer::Copy( _device, transferQueue, cmdPool, bufferSize, staggingBuffer, _content.buffer );
        

        // because the content of stagging buffer has been copying to vertex buffer,
        // the stagging buffer and memory buffer now not needed at all 
        vkDestroyBuffer( _device, staggingBuffer, nullptr );
        vkFreeMemory( _device, staggingBufferMemory, nullptr );


        // note :
        // I think in this function, the "Host" and "Device" is meaning this: 
        //      Host: is the CPU-GPU
        //      Device: is the GPU
    }



private:
    Content _content;
    std::array<VkBufferUsageFlagBits, 2> usagebufferlist = {
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,      // usagebuffer ke - 0
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT        // usagebuffer ke - 1
    };

    VkPhysicalDevice _physicalDevice;
    VkDevice _device;

/*
 *  from udemy :
 *      vertex input
 *      Index Buffer and Staging Buffer
*/
};