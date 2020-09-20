#include "HelloTriangleApp.h"

#include <exception>
#include <iostream>

void HelloTriangleApp::Run()
{
    InitVulkan();
    InitWindow();
    MainLoop();
    Cleanup();
}

void HelloTriangleApp::InitVulkan()
{

}

void HelloTriangleApp::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Triangle Application";
    appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t extensionsCount = 0U;
    const char** extension = glfwGetRequiredInstanceExtensions( &extensionsCount );

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.enabledExtensionCount = extensionsCount;
    instanceInfo.ppEnabledExtensionNames = extension;

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
    glfwDestroyWindow( _window );

    glfwTerminate();
}