#include "HelloTriangleApp.h"
#define _DEBUG

#include <exception>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <set>
#include <array>

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
    CreateImageViews();     // image views
    CreateRenderPass();     // render pass
    CreateGraphicsPipeline(); // graphics pipeline
    CreateFramebuffers();   // framebuffers (swapchain framebuffer images)
    CreateCommandPool();    // command pool

    CreateMeshFromVerteces();   // mesh

    CreateCommandBuffers(); // command buffers
    CreateSyncObjects();     // semaphores and fences
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
        DrawFrame();
    }

    vkQueueWaitIdle( _graphicsQueue );
    vkDeviceWaitIdle( _device );

/*
 *  from :
 *      Rendering and presentation -> Setup
 *      Rendering and presentation -> Presentation
 * 
*/
}

void HelloTriangleApp::Cleanup()
{
    mesh.DestroyMeshesContent();

    for( size_t i = 0; i < HelloTriangleApp::MaxFrameInFlight; ++i )
    {
        vkDestroySemaphore( _device, _renderFinishedSemaphore[i], nullptr );   // render finished semaphore
        vkDestroySemaphore( _device, _imageAvailableSemaphore[i], nullptr );   // image available semaphore
        vkDestroyFence( _device, _inFlightFences[i], nullptr ); // in fligh fence
    }

    vkDestroyCommandPool( _device, _commandPool, nullptr ); // command pool & command buffers

    for( auto& framebuffer : _swapchainFramebuffers )
    {
        vkDestroyFramebuffer( _device, framebuffer, nullptr );
    }

    vkDestroyPipeline( _device, _graphicsPipeline, nullptr );
    vkDestroyPipelineLayout( _device, _pipelineLayout, nullptr );   // pipeline layout
    vkDestroyRenderPass( _device, _renderPass, nullptr );

    for( auto& imageView : _swapchainImageViews )
    {
        vkDestroyImageView( _device, imageView, nullptr );  // image view
    }
    vkDestroySwapchainKHR( _device, _swapchain, nullptr );  // swapchain
    vkDestroyDevice( _device, nullptr );

    vkDestroySurfaceKHR( _instance, _surface, nullptr );

    if( enableValidationLayer )
        DestroyDebugUtilsMessengerEXT( _instance, _debugMessenger, nullptr );
    
    vkDestroyInstance( _instance, nullptr );
    glfwDestroyWindow( _window );

    glfwTerminate();
}


void HelloTriangleApp::DrawFrame()
{
    vkWaitForFences( _device, 1, &_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX );
    vkResetFences( _device, 1, &_inFlightFences[currentFrame] );

    uint32_t imageIndex;
    vkAcquireNextImageKHR( _device, _swapchain, UINT64_MAX, _imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex );

    // --- submitting the command buffer ---
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    std::array<VkSemaphore, 1> waitSemaphores = { _imageAvailableSemaphore[currentFrame] };
    std::array<VkPipelineStageFlags, 1> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>( waitSemaphores.size() );
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[imageIndex];

    std::array<VkSemaphore, 1> signalSemaphores = { _renderFinishedSemaphore[currentFrame] };
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>( signalSemaphores.size() );
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    ErrorCheck( vkQueueSubmit( _graphicsQueue, 1, &submitInfo, _inFlightFences[currentFrame] ), "submitting command buffer queue" );
    // --------------------------------------


    // --- presentation ---
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>( signalSemaphores.size() );
    presentInfo.pWaitSemaphores = signalSemaphores.data();

    std::array<VkSwapchainKHR, 1> swapchains = { _swapchain };
    presentInfo.swapchainCount = static_cast<uint32_t>( swapchains.size() );
    presentInfo.pSwapchains = swapchains.data();

    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    ErrorCheck( vkQueuePresentKHR( _presentQueue, &presentInfo ), "submitting the result back to swapchain to have it eventually show up to the screen" );
    // --------------------

    currentFrame = ++currentFrame % HelloTriangleApp::MaxFrameInFlight;

/*
 *  from :
 *      Rendering and presentation -> Setup & Synchronization
 *      Rendering and presentation -> Acquiring an image from the swapchain
 *      Rendering and presentation -> Submitting the command buffer
 *      Rendering and presentation -> Presentation
 *      Rendering and presentation -> In-Flight (almost done)
 * 
 *  from udemy :
 *      Drawing and Synchronization
 * 
*/
}

void HelloTriangleApp::CreateSyncObjects()
{
    // semaphore create info
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // fence create info
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // semaphore resize
    _imageAvailableSemaphore.resize( HelloTriangleApp::MaxFrameInFlight );
    _renderFinishedSemaphore.resize( HelloTriangleApp::MaxFrameInFlight );
    // fence resize
    _inFlightFences.resize( HelloTriangleApp::MaxFrameInFlight );

    for( size_t i = 0; i < HelloTriangleApp::MaxFrameInFlight; ++i )
    {
        ErrorCheck( vkCreateSemaphore( _device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore[i]), "create image available semaphores" );
        ErrorCheck( vkCreateSemaphore( _device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore[i]), "create render finished semaphore" );
        ErrorCheck( vkCreateFence( _device, &fenceInfo, nullptr, &_inFlightFences[i] ), "Creating fences" );
    }
    
// TODO
/*
 *  from :
 *      Rendering and presentation -> Semaphores
 *      Rendering and presentation -> In-Flight (almost done)
 * 
 *  from udemy :
 *      Drawing and Synchronization
 * 
*/
}


// --- Debug Messenger ---

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApp::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    std::cerr << "Validation Layer : " << pCallbackData->pMessage << std::endl << std::endl;
    return VK_FALSE;
}

void HelloTriangleApp::SetupDebugMessenger()
{
    if( !enableValidationLayer ) return;

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo;
    PopulateDebugUtilsCreateInfo( messengerInfo );

    ErrorCheck( CreateDebugUtilsMessengerEXT( _instance, &messengerInfo, nullptr, &_debugMessenger ), "create debug messenger" );
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
    ErrorCheck( glfwCreateWindowSurface( _instance, _window, nullptr, &_surface), "create surface");
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
    

    ErrorCheck( vkCreateDevice( _physicalDevice, &deviceInfo, nullptr, &_device ), "create logical device" );

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


// --- Mesh ---
void HelloTriangleApp::CreateMeshFromVerteces()
{
    // kotak

    vertices.resize(6);

    vertices[0] = {{ 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }};
    vertices[1] = {{ 0.5f, -0.5f, 0.0f }, {1.0f, 0.0f, 0.0f }};
    vertices[2] = {{ -0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 1.0f}};

    vertices[3] = {{ 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }};
    vertices[4] = {{ -0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 1.0f}};
    vertices[5] = {{ -0.5f, 0.5f, 0.0f }, {0.5f, 0.2f, 0.0f}};  


    // queue for transfer usually is the same as queue for graphics
    mesh = Mesh( _physicalDevice, _device, _graphicsQueue, _commandPool, vertices );
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

    ErrorCheck( vkCreateSwapchainKHR( _device, &swapchainInfo, nullptr, &_swapchain), "create swapchain");

    // retrive swapchain image
    vkGetSwapchainImagesKHR( _device, _swapchain, &imageCount /*we can re-use imageCount variable*/, nullptr );
    _swapchainImages.resize( imageCount );
    vkGetSwapchainImagesKHR( _device, _swapchain, &imageCount, _swapchainImages.data() );

    // store the format and the extent to class member variable.
    _swapchainImageFormat = surfaceFormat.format;
    _swapchainExtent = extent;


    /*
    *  from :
    *      Swapchain -> Creating the swapchain
    *      Swapchain -> Retrieving the swapchain image
    */
}

void HelloTriangleApp::CreateImageViews()
{
    _swapchainImageViews.resize( _swapchainImages.size() );

    for( int i = 0; i < _swapchainImageViews.size(); ++i )
    {
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = _swapchainImages[i];
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = _swapchainImageFormat;

        // component mapping
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    
        // image sub-resource range
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0U;
        imageViewInfo.subresourceRange.levelCount = 1U;
        imageViewInfo.subresourceRange.baseArrayLayer = 0U;
        imageViewInfo.subresourceRange.layerCount = 1U;

        if( vkCreateImageView( _device, &imageViewInfo, nullptr, &_swapchainImageViews[i] )
            != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to create image views!" );
        }
    }

    /*
    *   from :
    *       Image Views
    * 
    */
}



// --- Shader and Graphics Pipeline ---

void HelloTriangleApp::CreateRenderPass()
{
    // attachment description
    VkAttachmentDescription colorAttachmentDesc{};
    colorAttachmentDesc.format = _swapchainImageFormat;
    colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // attachment reference { layout( location = 0 ) out vec4 outColot}
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // subpass description
    VkSubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;

    // dependency
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // render pass create info
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachmentDesc;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    ErrorCheck( vkCreateRenderPass( _device, &renderPassInfo, nullptr, &_renderPass), "create render pass");
/*
 *  from :
 *      Render passes -> Attachment desciption
 *      Render passes -> Subpasses and attachment references
 *      Render passes -> Render pass
 *      Render and presentation -> Subpass dependencies
 */
}

void HelloTriangleApp::CreateGraphicsPipeline()
{
    // programable stage
    // -----------------

    auto vertShaderCode = ReadFile("shaders/vert.spv");
    auto fragShaderCode = ReadFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule( vertShaderCode );
    VkShaderModule fragShaderModule = CreateShaderModule( fragShaderCode );

    // Vertex Pipeline Stage Info
    VkPipelineShaderStageCreateInfo vertStageInfo{};
    vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = vertShaderModule;
    vertStageInfo.pName = "main";

    // Fragment Pipeline Stage Info
    VkPipelineShaderStageCreateInfo fragStageInfo{};
    fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = fragShaderModule;
    fragStageInfo.pName = "main";

    // Shader Stages
    // std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
    //     vertStageInfo, fragStageInfo
    // };
    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertStageInfo, fragStageInfo
    };

    // -----------------



    // fixed functions
    // --------------

    // vertex input
    auto bindDesc = GetBindingDescription();
    auto attDesc = GetAttributeDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = GetVertexInput( bindDesc, attDesc );

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = GetInputAssembly();
    
    // viewports & scissors
    VkViewport viewport = GetViewport();
    VkRect2D scissor = GetScissor();
    VkPipelineViewportStateCreateInfo viewportInfo = GetViewPortScissors( viewport, scissor );

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerInfo = GetRasterizer();

    // multisampling
    VkPipelineMultisampleStateCreateInfo multisampleInfo = GetMultisampling();
    
    // color blend
    VkPipelineColorBlendAttachmentState colorblendAttachment = GetColorBlendAttachment();
    VkPipelineColorBlendStateCreateInfo colorblendInfo = GetColorblending( colorblendAttachment );

    // depth and stencil testing (we don't need it for now)
    // 

    // dynamic state (we don't need it too for now)
    //

    // pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = GetPipelineLayout();
    ErrorCheck( vkCreatePipelineLayout( _device, &pipelineLayoutInfo, nullptr, &_pipelineLayout ), "create pipeline layout" );

    // --------------

    
    // Pipeline create info
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    // pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    // pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorblendInfo;
    // pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;   // index of a subpass in renderPass
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;    // because there are none base pipeline we want to use

    ErrorCheck( vkCreateGraphicsPipelines( _device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline), "create graphics pipeline" );

    // destroy shader module
    vkDestroyShaderModule( _device, vertShaderModule, nullptr );
    vkDestroyShaderModule( _device, fragShaderModule, nullptr );

/*
 *  from :
 *      Graphics Pipeline Introduction
 *      Shader modules -> Loading a shader
 *      Shader modules -> Creating shader module
 *      Shader modules -> Shader stage creation
 *      Fixed Functions (from vertex input, until pipeline layout)
 * 
 * 
 *  from udemy :
 *      vertex input
 * 
 */
}

std::vector<char> HelloTriangleApp::ReadFile( const std::string& filename )
{
    std::ifstream in( filename, std::ios::ate | std::ios::binary );
    
    if( !in.is_open() )
        throw std::runtime_error( "Failed to open spv file!" );
    
    size_t fileSize = static_cast<size_t>( in.tellg() ); // getting file size
    std::vector<char> buffer( fileSize ); // this vector<char> will be used for storing the content of spv file

    in.seekg( 0 );  // making file read from begining
    in.read( buffer.data(), fileSize ); // read file and then store it at buffer

    in.close();

    return buffer;

/*
 *  from :
 *      Shader modules -> Loading a shader
 *      Conclusion (conclusion of all steps in graphics pipeline section)
 */
}

VkShaderModule HelloTriangleApp::CreateShaderModule( const std::vector<char>& code )
{
    VkShaderModuleCreateInfo moduleInfo{};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = code.size();
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>( code.data() );

    VkShaderModule shaderModule;
    ErrorCheck( vkCreateShaderModule( _device, &moduleInfo, nullptr, &shaderModule ), " create shader module" );

    return shaderModule;

/*
 *  from :
 *      Shader modules -> Creating shader module
 * 
 */
}

VkPipelineVertexInputStateCreateInfo HelloTriangleApp::GetVertexInput(
    VkVertexInputBindingDescription& bindDesc, 
    std::vector<VkVertexInputAttributeDescription>& attDesc
    )
{
    VkPipelineVertexInputStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    createInfo.vertexBindingDescriptionCount = 1;
    createInfo.pVertexBindingDescriptions = &bindDesc;
    createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attDesc.size());
    createInfo.pVertexAttributeDescriptions = attDesc.data();

    return createInfo;
/*
 *  from :
    // color blend
    VkPipelineColorBlendStateCreateInfo colorblendInfo = GetColorblending();
 *      Fixed functions -> Vertex input
 * 
 * 
 *  from udemy :
 *      vertex input
 * 
 */
}

VkVertexInputBindingDescription HelloTriangleApp::GetBindingDescription()
{
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(Vertex);
    bindingDesc.inputRate =  VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDesc;
     
/*
 *  from udemy :
 *      vertex input
*/
}

std::vector<VkVertexInputAttributeDescription> HelloTriangleApp::GetAttributeDescription()
{
    std::vector<VkVertexInputAttributeDescription> attDesc( 2 );

    // position attribute
    attDesc[0].location = 0;
    attDesc[0].binding = 0;
    attDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attDesc[0].offset = offsetof(Vertex, pos);

    // color attribute
    attDesc[1].location = 1;
    attDesc[1].binding = 0;
    attDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attDesc[1].offset = offsetof(Vertex, col);

    return attDesc;
     
/*
 *  from udemy :
 *      vertex input
*/
}

VkPipelineInputAssemblyStateCreateInfo HelloTriangleApp::GetInputAssembly()
{
    VkPipelineInputAssemblyStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    createInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    createInfo.primitiveRestartEnable = VK_FALSE;

    return createInfo;
/*
 *  from :
 *      Fixed functions -> Input assembly
 * 
 */
}

VkPipelineViewportStateCreateInfo HelloTriangleApp::GetViewPortScissors( VkViewport& viewport, VkRect2D& scissor )
{
    // viewport + scissor
    VkPipelineViewportStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createInfo.viewportCount = 1;       // jumlah viewport
    createInfo.pViewports = &viewport;  // viewport
    createInfo.scissorCount = 1;        // jumlah scissor
    createInfo.pScissors = &scissor;    // scissor

    return createInfo;
/*
 *  from :
 *      Fixed functions -> Viewports and scissors
 * 
 */
}

VkPipelineRasterizationStateCreateInfo HelloTriangleApp::GetRasterizer()
{
    VkPipelineRasterizationStateCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    
    // depth clamp
    createInfo.depthClampEnable = VK_FALSE;
    
    // discard
    createInfo.rasterizerDiscardEnable = VK_FALSE;
    
    // polygon mode
    createInfo.polygonMode = VK_POLYGON_MODE_FILL;
    
    // line width
    createInfo.lineWidth = 1.0f;
    
    // cull mode + front face
    createInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    createInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    
    // depth bias
    createInfo.depthBiasEnable = VK_FALSE;
    // createInfo.depthBiasConstantFactor = 0.0f;  // optional
    // createInfo.depthBiasClamp = 0.0f;   // optional
    // createInfo.depthBiasSlopeFactor = 0.0f; // optional

    return createInfo;
/*
 *  from :
 *      Fixed functions -> Rasterizer
 * 
 */
}

VkPipelineMultisampleStateCreateInfo HelloTriangleApp::GetMultisampling()
{
    VkPipelineMultisampleStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    createInfo.sampleShadingEnable = VK_FALSE;
    createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    // createInfo.minSampleShading = 1.0f; // optional
    // createInfo.pSampleMask = nullptr;   // optional
    // createInfo.alphaToCoverageEnable = VK_FALSE;    // optional
    // createInfo.alphaToOneEnable = VK_FALSE; // optional

    return createInfo;
/*
 *  from :
 *      Fixed functions -> Multisampling
 * 
 */
}

VkPipelineColorBlendStateCreateInfo HelloTriangleApp::GetColorblending( VkPipelineColorBlendAttachmentState& attachment )
{
    VkPipelineColorBlendStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    createInfo.logicOpEnable = VK_FALSE;
    createInfo.logicOp = VK_LOGIC_OP_COPY;  // optional
    createInfo.attachmentCount = 1;         // jumlah attachment
    createInfo.pAttachments = &attachment;  // attachment
    createInfo.blendConstants[0] = 0.0f;    // optional
    createInfo.blendConstants[1] = 0.0f;    // optional
    createInfo.blendConstants[2] = 0.0f;    // optional
    createInfo.blendConstants[3] = 0.0f;    // optional

    return createInfo;
/*
 *  from :
 *      Fixed functions -> Color blending
 * 
 */
}

VkPipelineLayoutCreateInfo HelloTriangleApp::GetPipelineLayout()
{
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 0;  // optional
    // createInfo.pSetLayouts = nullptr;   // optional
    createInfo.pushConstantRangeCount = 0;  // optional
    // createInfo.pPushConstantRanges = nullptr;   // optional

    return createInfo;
/*
 *  from :
 *      Fixed functions -> Pipeline layout
 * 
 */
}

VkViewport HelloTriangleApp::GetViewport() const
{
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = float(_swapchainExtent.width);
    viewport.height = float(_swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    return viewport;
}

VkRect2D HelloTriangleApp::GetScissor() const
{
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = _swapchainExtent;

    return scissor;
}

VkPipelineColorBlendAttachmentState HelloTriangleApp::GetColorBlendAttachment() const
{
    VkPipelineColorBlendAttachmentState colorAttachment{};
    colorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorAttachment.blendEnable = VK_FALSE;
    colorAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    return colorAttachment;
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


// --- command buffer and frame buffer ---

void HelloTriangleApp::CreateFramebuffers()
{
    const size_t size_images = _swapchainImageViews.size();
    _swapchainFramebuffers.resize( size_images );

    for( size_t i = 0; i < size_images; ++i )
    {
        std::array<VkImageView, 1> attachment = {
            _swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>( attachment.size() );
        framebufferInfo.pAttachments = attachment.data();
        framebufferInfo.width = _swapchainExtent.width;
        framebufferInfo.height = _swapchainExtent.height;
        framebufferInfo.layers = 1;

        ErrorCheck( vkCreateFramebuffer( _device, &framebufferInfo, nullptr, &_swapchainFramebuffers[i] ), "create framebuffer" );
    }
}

void HelloTriangleApp::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies( _physicalDevice );

    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags = 0;  // optional
    commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    ErrorCheck( vkCreateCommandPool( _device, &commandPoolInfo, nullptr, &_commandPool), "create command pool" );

}

void HelloTriangleApp::CreateCommandBuffers()
{   
    _commandBuffers.resize( _swapchainFramebuffers.size() );
    const size_t size_commandBuffers = _commandBuffers.size();
    
    VkCommandBufferAllocateInfo cmdAllocInfo{};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.commandPool = _commandPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = static_cast<uint32_t>( size_commandBuffers );

    ErrorCheck( vkAllocateCommandBuffers( _device, &cmdAllocInfo, _commandBuffers.data() ), "allocate command buffers" );

    for( size_t i = 0; i < size_commandBuffers; ++i )
    {
        // --- begin ---
        VkCommandBufferBeginInfo cmdBuf_beginInfo{};
        cmdBuf_beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBuf_beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;  // before this it's assgin to zero, but someone in the chat said that it can fixed issue in rendering & presentation
        cmdBuf_beginInfo.pInheritanceInfo = nullptr;    // optional

        ErrorCheck( vkBeginCommandBuffer( _commandBuffers[i], &cmdBuf_beginInfo ), "begin recording command buffer" );
        // -------------

        // --- render pass ---
        VkRenderPassBeginInfo renderpassBeginInfo{};
        renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpassBeginInfo.renderPass = _renderPass;
        renderpassBeginInfo.framebuffer = _swapchainFramebuffers[i];
        renderpassBeginInfo.renderArea.offset = {0, 0};
        renderpassBeginInfo.renderArea.extent = _swapchainExtent;
        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }; // solid black
        renderpassBeginInfo.clearValueCount = 1;
        renderpassBeginInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass( _commandBuffers[i], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

        // --- basic draw command ---
        // bind pipeline
        vkCmdBindPipeline( _commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline );
        
        // bind vertex buffer
        std::array<VkBuffer, 1> vertexBuffers = { mesh.GetVertexBuffer() };
        std::array<VkDeviceSize, 1> offsets = { 0 };
        vkCmdBindVertexBuffers( _commandBuffers[i], 0, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), offsets.data() );
        
        // draw
        // vkCmdDraw( _commandBuffers[i], 3, 1, 0, 0 );     // before
        vkCmdDraw( _commandBuffers[i], static_cast<uint32_t>(mesh.GetVertexCount()), 1, 0, 0 );        // after
        // --------------------------

        // --- Finish recording ---
        vkCmdEndRenderPass( _commandBuffers[i] );
        ErrorCheck( vkEndCommandBuffer( _commandBuffers[i] ), "end recording command buffer" );
    }

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

void HelloTriangleApp::ErrorCheck( VkResult result, const char* msg ) const
{
    std::string str = "Failed to ";
    str.append( msg );
    if( result != VK_SUCCESS )
    {
        throw std::runtime_error( str );
    }
}