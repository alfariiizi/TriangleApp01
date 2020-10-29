#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <memory.h>

#include "utilities.h"

class Mesh
{
public:
    Mesh() = default;
    Mesh( VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool cmdPool ,std::vector<Vertex>& vertices )
        :
        _physicalDevice( physicalDevice ),
        _device( device )
    {
        _vertexCount = (int)vertices.size();
        CreateVertexBuffer( transferQueue, cmdPool, vertices );
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
    void CreateVertexBuffer( VkQueue transferQueue, VkCommandPool cmdPool, std::vector<Vertex>& vertices )
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
        VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VkMemoryPropertyFlags memPropFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        
        Buffer::Create( _physicalDevice, _device, bufferSize, bufferUsage,
                               memPropFlags, _vertexBuffer, _vertexBufferMemory );
        // ----------------------------------

        // copying stagging buffer to vertex buffer
        Buffer::Copy( _device, transferQueue, cmdPool, bufferSize, staggingBuffer, _vertexBuffer );
        

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
    int _vertexCount;
    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;

    VkPhysicalDevice _physicalDevice;
    VkDevice _device;

/*
 *  from udemy :
 *      vertex input
 *      Index Buffer and Staging Buffer
*/
};