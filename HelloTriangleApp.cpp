#include "HelloTriangleApp.h"
#define _DEBUG

#include <exception>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>

#ifdef _DEBUG
const bool enableValidationLayer = true;
#else
const bool enableValidationLayer = false;
#endif

void HelloTriangleApp::Run()
{
    InitVulkan();
    InitWindow();
    MainLoop();
    Cleanup();
}

void HelloTriangleApp::InitVulkan()
{
    CreateInstance();

    if( enableValidationLayer ) SetupDebugMessenger();
}

void HelloTriangleApp::CreateInstance()
{
    if( enableValidationLayer && !CheckValidationErrorSupport())
        throw std::runtime_error( "Validation Layer requested, but not available!" );

    VkApplicationInfo appInfo;
    PopulateApplicationInfo( appInfo );

    auto extensions = GetRequiredExtensions();

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>( extensions.size() );
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo;
    if( enableValidationLayer )
    {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerExtension.size());
        instanceInfo.ppEnabledLayerNames = validationLayerExtension.data();
        
        PopulateDebugUtilsCreateInfo( messengerInfo );
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengerInfo;
    }
    else
    {
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.ppEnabledLayerNames = nullptr;
    }

    if( vkCreateInstance( &instanceInfo, nullptr, &_instance ) != VK_SUCCESS )
        throw std::runtime_error( "Failed to create instance!" );
}

void HelloTriangleApp::InitWindow()
{
    glfwInit();
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    _window = glfwCreateWindow( ScreenWidth, ScreenHeight, 
                                "Triangle App", nullptr, nullptr );
    
}

void HelloTriangleApp::MainLoop()
{
    while( !glfwWindowShouldClose( _window ) )
    {
        glfwPollEvents();
    }
}

void HelloTriangleApp::Cleanup()
{
    if( enableValidationLayer )
        DestroyDebugUtilsMessengerEXT( _instance, _debugMessenger, nullptr );
    
    vkDestroyInstance( _instance, nullptr );
    glfwDestroyWindow( _window );

    glfwTerminate();
}



// --- Debug Messenger ---

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApp::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    std::cerr << "Validation Layer : " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void HelloTriangleApp::SetupDebugMessenger()
{
    if( !enableValidationLayer ) return;

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo;
    PopulateDebugUtilsCreateInfo( messengerInfo );

    if( CreateDebugUtilsMessengerEXT( _instance, &messengerInfo, nullptr, &_debugMessenger ) 
        != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create debug messenger!" );
    }
}

VkResult HelloTriangleApp::CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if( func != nullptr )
    {
        return func( instance, pCreateInfo, pAllocator, pDebugMessenger );
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void HelloTriangleApp::DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
)
{
    auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
    if( func != nullptr )
        func( instance, debugMessenger, pAllocator );
}


// --- Physical Device ---
void HelloTriangleApp::PickPhysicalDevice()
{
    uint32_t deviceCount = 0U;
    vkEnumeratePhysicalDevices( _instance, &deviceCount, nullptr );
    std::vector<VkPhysicalDevice> physicalDevices( deviceCount );
    vkEnumeratePhysicalDevices( _instance, &deviceCount, physicalDevices.data() );

    for( const auto& physicalDevice : physicalDevices )
    {
        if( IsDeviceSuitable( physicalDevice ) )
        {
            _physicalDevice = physicalDevice;
            break;
        }
    }

    if( _physicalDevice == VK_NULL_HANDLE )
        throw std::runtime_error( "Failed to find suitable physical device!" );

}

bool HelloTriangleApp::IsDeviceSuitable( VkPhysicalDevice physicalDevice )
{
    // VkPhysicalDeviceProperties properties;
    // vkGetPhysicalDeviceProperties( physicalDevice, &properties );
    // VkPhysicalDeviceFeatures features;
    // vkGetPhysicalDeviceFeatures( physicalDevice, &features );
    // + Some code to rateDeviceSuitability or maybe just pick suitabledevice that you needed
    
    // I just need vulkan works LOL, so I leave it like this.
    QueueFamilyIndices indices = FindQueueFamilies( _physicalDevice );

    return indices.IsComplete();
}

// --- Queue Families ---
QueueFamilyIndices HelloTriangleApp::FindQueueFamilies( VkPhysicalDevice physicalDevice )
{
    QueueFamilyIndices indices;

    // logic to find queue families
    uint32_t queueFamilyCount = 0U;
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, nullptr );
    std::vector<VkQueueFamilyProperties> propertiesQueueFamily( queueFamilyCount );
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, propertiesQueueFamily.data() );

    int i = 0;
    for( const auto& property : propertiesQueueFamily )
    {
        if( property.queueFlags & VK_QUEUE_GRAPHICS_BIT )
            indices.graphicsFamily = i;
        
        // ntar disini diisi suatu code untuk queue family lainnya.

        if( indices.IsComplete() )
            break;
    }

    return indices;
}




// --- Getter Method ---

std::vector<const char*> HelloTriangleApp::GetRequiredExtensions()
{
    uint32_t glfwExtensionsCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionsCount );

    std::vector<const char*> extensions( glfwExtensions, glfwExtensions + glfwExtensionsCount );
    
    extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

    return extensions;
}



// --- Cheker Method ---

bool HelloTriangleApp::CheckValidationErrorSupport()
{
    uint32_t layerCount = 0U;
    vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
    std::vector<VkLayerProperties> layer( layerCount );
    vkEnumerateInstanceLayerProperties( &layerCount, layer.data() );
    
    bool isFound = false;
    for( const auto& v : validationLayerExtension )
    {
        if( std::find_if( layer.begin(), layer.end(), 
            [v](VkLayerProperties l){ return strcmp( v, l.layerName ); } )
            != layer.end() )
        {
            isFound = true;
            break;
        }

    }

    return isFound;
}




// Populate Method

void HelloTriangleApp::PopulateApplicationInfo( VkApplicationInfo& appInfo )
{
    appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Triangle Application";
    appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.apiVersion = VK_API_VERSION_1_0;
}

void HelloTriangleApp::PopulateDebugUtilsCreateInfo( VkDebugUtilsMessengerCreateInfoEXT& createInfo )
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    createInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr; //optional
}