#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <fstream>

const std::vector<const char*> validationLayerExtension {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME // this extensions is for swapchain support
};

#include "utilities.h"
#include "Mesh.h"


class HelloTriangleApp
{
public:
    void Run();

private:
// Main Method
    void InitVulkan();
    void CreateInstance();
    void InitWindow();
    void MainLoop();
    void Cleanup();

// Debug
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT typeSeverity,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );
    void SetupDebugMessenger();
    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
    );
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    );
    bool CheckValidationErrorSupport();

// Surface
    void CreateSurface();

// Physical Device
    void PickPhysicalDevice();
    bool IsDeviceSuitable( VkPhysicalDevice physicalDevice );

// Queue Families
    QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice physicalDevice );

// Logical Device
    void CreateLogicalDevice();

// mesh
    void CreateMeshFromVerteces();

// Swapchain
    SwapchainSupportDetails QuerySwapchainSupport( VkPhysicalDevice physicalDevice );
    VkExtent2D ChooseSwapchainExtent2D( const VkSurfaceCapabilitiesKHR& capabilities );
    VkSurfaceFormatKHR ChooseSwapchainFormat( const std::vector<VkSurfaceFormatKHR>& avaliableFormats );
    VkPresentModeKHR ChooseSwapchainPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes );
    void CreateSwapchain();
    void CreateImageViews();

// Shader and Graphics Pipeline
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    static std::vector<char> ReadFile( const std::string& filename );
    VkShaderModule CreateShaderModule( const std::vector<char>& code );
    VkPipelineVertexInputStateCreateInfo GetVertexInput(
        VkVertexInputBindingDescription& bindDesc, 
        std::vector<VkVertexInputAttributeDescription>& attDesc
        );
    VkVertexInputBindingDescription GetBindingDescription();
    std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
    VkPipelineInputAssemblyStateCreateInfo GetInputAssembly();
    VkPipelineViewportStateCreateInfo GetViewPortScissors( VkViewport& viewport, VkRect2D& scissor );
    VkPipelineRasterizationStateCreateInfo GetRasterizer();
    VkPipelineMultisampleStateCreateInfo GetMultisampling();
    VkPipelineColorBlendStateCreateInfo GetColorblending( VkPipelineColorBlendAttachmentState& attachment );
    VkPipelineLayoutCreateInfo GetPipelineLayout();

    // Getter Function for Fixed Function in Graphics Pipeline
    VkViewport GetViewport() const;
    VkRect2D GetScissor() const;
    VkPipelineColorBlendAttachmentState GetColorBlendAttachment() const;


// command buffer and frame buffer
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();


// rendering and presentation
    void DrawFrame();
    void CreateSyncObjects();

// Eextensions
    std::vector<const char*> GetRequiredExtensions();
    bool CheckDeviceExtensionSupport( VkPhysicalDevice physicalDevice );


// Populate Method
    void PopulateApplicationInfo( VkApplicationInfo& appInfo );
    void PopulateDebugUtilsCreateInfo( VkDebugUtilsMessengerCreateInfoEXT& createInfo );

// Error Check
    void ErrorCheck( VkResult result, const char* msg ) const;


public:
    static constexpr int ScreenWidth = 800;
    static constexpr int ScreenHeight = 600;
    static constexpr int MaxFrameInFlight = 2;
private:
    size_t currentFrame = 0;   // this variable is for keep tracking the frame in flight

private:
    GLFWwindow* _window = nullptr;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device;

    // mesh
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    Mesh _vertexMesh;
    Mesh _indexMesh;

    // queue
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    // presentation
    VkSurfaceKHR _surface;

    // swapchain
    VkSwapchainKHR _swapchain;
    std::vector<VkImage> _swapchainImages;  // swapchain image
    std::vector<VkImageView> _swapchainImageViews;   // swapchain image views
    VkFormat _swapchainImageFormat;     // swapchain format
    VkExtent2D _swapchainExtent;        // swapchain extent

    // graphics pipeline section
    VkRenderPass _renderPass;   // render pass
    VkPipelineLayout _pipelineLayout;   // pipeline layout (see: fixed function)
    VkPipeline _graphicsPipeline;   // graphics pipeline

    // command buffer and frame buffer section
    std::vector<VkFramebuffer> _swapchainFramebuffers;
    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;

    // semaphores
    std::vector<VkSemaphore> _imageAvailableSemaphore;
    std::vector<VkSemaphore> _renderFinishedSemaphore;
    // fences
    std::vector<VkFence> _inFlightFences;
};