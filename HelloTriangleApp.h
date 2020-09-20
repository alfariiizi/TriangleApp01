#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class HelloTriangleApp
{
public:
    void Run();

private:
    void InitVulkan();
    void CreateInstance();
    void InitWindow();
    void MainLoop();
    void Cleanup();

public:
    static constexpr int ScreenWidth = 800;
    static constexpr int ScreenHeight = 600;

private:
    GLFWwindow* _window = nullptr;
    VkInstance _instance;
};