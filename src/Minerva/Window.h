#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Minerva
{
    class Window
    {
        
    public:
        const int WIDTH = 1920, HEIGHT = 1080;
        GLFWwindow* window;
        Window() = default;
        ~Window();
        void EngineInitWindow(int width, int height);
    };
}


