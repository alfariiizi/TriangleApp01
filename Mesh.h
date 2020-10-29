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
        size_t bufferSize = sizeof(Vertex) * vertices.size();
        VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VkMemoryPropertyFlags memPropFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        
        Buffer::CreateBuffer( _physicalDevice, _device, bufferSize, bufferUsage,
                               memPropFlags, _vertexBuffer, _vertexBufferMemory );
        

        // mapping the memory (yang baru saja dialokasikan) to vertex buffer
        void * data;
        vkMapMemory( _device, _vertexBufferMemory, 0, bufferSize, 0, &data );
        memcpy( data, vertices.data(), size_t(bufferSize) );
        vkUnmapMemory( _device, _vertexBufferMemory );
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