#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

const std::vector<const char*> validationLayerExtension {
    "VK_LAYER_KHRONOS_validation"
};

#include "QueueFamilyIndices.h"


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


// Physical Device
    void PickPhysicalDevice();
    bool IsDeviceSuitable( VkPhysicalDevice physicalDevice );

//Queue Families
    QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice physicalDevice );


// General Getter Method
    std::vector<const char*> GetRequiredExtensions();


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
};