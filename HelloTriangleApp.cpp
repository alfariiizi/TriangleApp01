#include "HelloTriangleApp.h"

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

void HelloTriangleApp::InitWindow()
{
    glfwInit();
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    window = glfwCreateWindow( ScreenWidth, ScreenHeight, 
                                "Triangle App", nullptr, nullptr );
    
}

void HelloTriangleApp::MainLoop()
{
    while( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();
    }
}

void HelloTriangleApp::Cleanup()
{
    glfwDestroyWindow( window );
}