#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

const std::vector<const char*> validationLayerExtension {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME // this extensions is for swapchain support
};

#include "QueueFamilyIndices.h"
#include "SwapchainSupportDetails.h"


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

// Swapchain
    SwapchainSupportDetails QuerySwapchainSupport( VkPhysicalDevice physicalDevice );
    VkExtent2D ChooseSwapchainExtent2D( const VkSurfaceCapabilitiesKHR& capabilities );
    VkSurfaceFormatKHR ChooseSwapchainFormat( const std::vector<VkSurfaceFormatKHR>& avaliableFormats );
    VkPresentModeKHR ChooseSwapchainPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes );
    void CreateSwapchain();



// Eextensions
    std::vector<const char*> GetRequiredExtensions();
    bool CheckDeviceExtensionSupport( VkPhysicalDevice physicalDevice );


// Populate Method
    void PopulateApplicationInfo( VkApplicationInfo& appInfo );
    void PopulateDebugUtilsCreateInfo( VkDebugUtilsMessengerCreateInfoEXT& createInfo );



public:
    static constexpr int ScreenWidth = 800;
    static constexpr int ScreenHeight = 600;

private:
    GLFWwindow* _window = nullptr;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device;

    // queue
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    // presentation
    VkSurfaceKHR _surface;

    // swapchain
    VkSwapchainKHR _swapchain;
};