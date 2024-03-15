#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Minerva
{
    class Window
    {
        
    public:
        const int WIDTH = 600, HEIGHT = 600;
        GLFWwindow* window;
        Window() = default;
        ~Window();
        /// @brief Create and initialize the window according to some params
        /// @param width Is the width of window 
        /// @param height Is the height of window
        void EngineInitWindow(int width, int height);
    };
}


