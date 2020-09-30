#include "HelloTriangleApp.h"
#define _DEBUG

#include <exception>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <set>

#ifdef _DEBUG
const bool enableValidationLayer = true;
#else
const bool enableValidationLayer = false;
#endif

void HelloTriangleApp::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void HelloTriangleApp::InitVulkan()
{
    CreateInstance();
    if( enableValidationLayer ) SetupDebugMessenger();
    CreateSurface();    // surface

    PickPhysicalDevice();   // physical device
    CreateLogicalDevice();  // logical device
    CreateSwapchain();      // swapchain
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
    vkDestroySwapchainKHR( _device, _swapchain, nullptr );  // swapchain
    vkDestroyDevice( _device, nullptr );

    vkDestroySurfaceKHR( _instance, _surface, nullptr );

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



// --- Surface ---
void HelloTriangleApp::CreateSurface()
{
    if( glfwCreateWindowSurface( _instance, _window, nullptr, &_surface)
        !=
        VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create surface!" );
    }
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
    QueueFamilyIndices indices = FindQueueFamilies( physicalDevice );

    bool isExtensionSupported = CheckDeviceExtensionSupport( physicalDevice );

    bool swapChainAdequate = false;
    if( isExtensionSupported )
    {
        SwapchainSupportDetails details = QuerySwapchainSupport( physicalDevice );
        swapChainAdequate = !details.IsEmpty();
    }

    return indices.IsComplete() && isExtensionSupported && swapChainAdequate;

    /*
    *   from :
    *       Swapchain -> Checking for swapchain support
    *       Swapchain -> Querying details of swapchain support 
    */
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
        // nah ini queue family lainnya :
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, i, _surface, &presentSupport );
        if( presentSupport )
            indices.presentFamily = i;

        if( indices.IsComplete() )
            break;
    }

    return indices;
}



// --- Logical Device ---
void HelloTriangleApp::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies( _physicalDevice );

    
    std::vector<VkDeviceQueueCreateInfo> logDevQueueInfos;  // queue infos
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };    
    float queuePriority = 1.0f;

    for( auto& queueFamily : uniqueQueueFamilies )
    {
        VkDeviceQueueCreateInfo logDevQueueInfo{}; // logical device queue create info
        logDevQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        logDevQueueInfo.pNext = nullptr;
        logDevQueueInfo.queueCount = 1;
        logDevQueueInfo.queueFamilyIndex = queueFamily;
        logDevQueueInfo.pQueuePriorities = &queuePriority;

        logDevQueueInfos.push_back(logDevQueueInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    /*
     * ntar deviceFeatures nya kita isi disini
     */

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>( logDevQueueInfos.size() ); // jumlah queue family
    deviceInfo.pQueueCreateInfos = logDevQueueInfos.data(); // vector queue family crete infos nya
    deviceInfo.pEnabledFeatures = &deviceFeatures;  // device features nya

    if( enableValidationLayer )
    {
        deviceInfo.enabledLayerCount = static_cast<uint32_t>( validationLayerExtension.size() );
        deviceInfo.ppEnabledLayerNames = validationLayerExtension.data();
    }
    else
        deviceInfo.enabledLayerCount = 0;
    
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>( deviceExtensions.size() );
    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
    

    if( vkCreateDevice( _physicalDevice, &deviceInfo, nullptr, &_device )
        !=
        VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create logical device!" );
    }

    // create queue
    vkGetDeviceQueue( _device, indices.graphicsFamily.value(), 0, &_graphicsQueue );
    vkGetDeviceQueue( _device, indices.presentFamily.value(), 0, &_presentQueue );


    /*
     *  from :
     *      ...
     *      ...
     *      Swapchain -> Enabling device extensions
     */


}


// --- Swapchain ---
SwapchainSupportDetails HelloTriangleApp::QuerySwapchainSupport( VkPhysicalDevice physicalDevice )
{
    SwapchainSupportDetails details;

    // surface capabilities :
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, _surface, &details.surfaceCapabilities );

    // surface formats :
    uint32_t formatCount = 0U;
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, _surface, &formatCount, nullptr );
    details.formats.resize( formatCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, _surface, &formatCount, details.formats.data() );

    // presentations modes :
    uint32_t presentCount = 0U;
    vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, _surface, &presentCount, nullptr );
    details.presentModes.resize( presentCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, _surface, &presentCount, details.presentModes.data() );

    return details;

    /*
    *  from :
    *      Swapchain -> Querying details of swapchain support 
    */
}

VkExtent2D HelloTriangleApp::ChooseSwapchainExtent2D( const VkSurfaceCapabilitiesKHR& capabilities )
{
    if( capabilities.currentExtent.width != UINT32_MAX )
        return capabilities.currentExtent;
    
    
    VkExtent2D actualExtent = { HelloTriangleApp::ScreenWidth, HelloTriangleApp::ScreenHeight };

    actualExtent.width = std::clamp( actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
    actualExtent.height = std::clamp( actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );

    return actualExtent;

    /*
    *  from :
    *      Swapchain -> Choosing the right settings for the swapchain -> Swap Extent 
    */
}

VkSurfaceFormatKHR HelloTriangleApp::ChooseSwapchainFormat( const std::vector<VkSurfaceFormatKHR>& avaliableFormats )
{
    for( const auto& avaliableFormat : avaliableFormats )
    {
        if( avaliableFormat.format == VK_FORMAT_B8G8R8_SRGB &&
            avaliableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
        {
            return avaliableFormat;
        }
    }
    return avaliableFormats[0];

    /*
    *  from :
    *      Swapchain -> Choosing the right settings for the swapchain -> Surface Format
    */
}

VkPresentModeKHR HelloTriangleApp::ChooseSwapchainPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes )
{
    for( const auto& availablePresentMode : availablePresentModes )
    {
        if( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
            return availablePresentMode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;

    /*
    *  from :
    *      Swapchain -> Choosing the right settings for the swapchain -> Presentation Mode
    */
}

void HelloTriangleApp::CreateSwapchain()
{
    SwapchainSupportDetails details = QuerySwapchainSupport( _physicalDevice );
    
    VkExtent2D extent = ChooseSwapchainExtent2D( details.surfaceCapabilities );
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainFormat( details.formats );
    VkPresentModeKHR presentMode = ChooseSwapchainPresentMode( details.presentModes );

    uint32_t imageCount = details.surfaceCapabilities.minImageCount + 1;
    if( details.surfaceCapabilities.maxImageCount > 0 &&
        imageCount > details.surfaceCapabilities.maxImageCount )
    {
        imageCount = details.surfaceCapabilities.maxImageCount;
    }


    // ***Create Info***
    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    // surface
    swapchainInfo.surface = _surface;
    // min image count
    swapchainInfo.minImageCount = imageCount;
    // image format
    swapchainInfo.imageFormat = surfaceFormat.format;
    // image color space
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    // image extent
    swapchainInfo.imageExtent = extent;
    // image array layers
    swapchainInfo.imageArrayLayers = 1;
    // image usage
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // image sharing mode
    QueueFamilyIndices indices = FindQueueFamilies( _physicalDevice );
    uint32_t indicesArr[] /*indices in array */ = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    if( indicesArr[0] != indicesArr[1] )
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = static_cast<uint32_t>( sizeof( indicesArr ) );
        swapchainInfo.pQueueFamilyIndices = indicesArr;
    }
    else
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices = nullptr;
    }
    // pre transform
    swapchainInfo.preTransform = details.surfaceCapabilities.currentTransform;
    // composite alpha
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // presentation mode
    swapchainInfo.presentMode = presentMode;
    // clipped
    swapchainInfo.clipped = VK_TRUE;
    // old swap chain
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    // ***************

    if( vkCreateSwapchainKHR( _device, &swapchainInfo, nullptr, &_swapchain)
        != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create swapchain!" );
    }

    /*
    *  from :
    *      Swapchain -> Creating the swapchain
    */
}




// --- Extensions ---

std::vector<const char*> HelloTriangleApp::GetRequiredExtensions()
{
    uint32_t glfwExtensionsCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionsCount );

    std::vector<const char*> extensions( glfwExtensions, glfwExtensions + glfwExtensionsCount );
    
    extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

    return extensions;
}

bool HelloTriangleApp::CheckDeviceExtensionSupport( VkPhysicalDevice physicalDevice )
{
    uint32_t extensionCount = 0U;
    vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionCount, nullptr );
    std::vector<VkExtensionProperties> avaliableExtensions( extensionCount );
    vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionCount, avaliableExtensions.data() );

    std::set<std::string> requiredExtensions { deviceExtensions.begin(), deviceExtensions.end() };

    for( const auto& extension : avaliableExtensions )
    {
        requiredExtensions.erase( extension.extensionName );
    }

    return requiredExtensions.empty();

    /*
    * from :
    *   Swapchain -> Checking for swapchain support
    * 
    */
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