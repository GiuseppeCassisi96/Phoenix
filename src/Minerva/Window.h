#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "vector"


namespace Minerva
{
    class Window
    {
        
    public:
        const int WIDTH = 600, HEIGHT = 600;
        VkSurfaceKHR windowSurface = VK_NULL_HANDLE;  
        GLFWwindow* window = nullptr;
        Window() = default;
        ~Window();
        /// @brief Create and initialize the window according to some params
        /// @param width Is the width of window 
        /// @param height Is the height of window
        void EngineInitWindow(int width, int height);
        void CreateWindowSurface(const VkInstance& instance);
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

    };
}


