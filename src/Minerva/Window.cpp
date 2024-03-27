#include "Window.h"
#include <stdexcept>
#include <iostream>
#include "Renderer.h"
#include "EngineVars.h"
namespace Minerva
{
    void Window::EngineInitWindow(int width, int height)
    {
        glfwInit();
        /*Because GLFW was originally designed to create an OpenGL context, we need to tell it to not
        *create an OpenGL context with a subsequent call*/
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        /*Because handling resized windows takes special care that we'll look into later, disable it for
        *now with another window hint call*/
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        /*All that's left now is creating the actual window. Add a GLFWwindow*
        *window; private class member to store a reference to it and initialize the window with:*/
        window = glfwCreateWindow(width, width, "Minerva", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
    }

    void Window::CreateWindowSurface(const VkInstance &instance)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &windowSurface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void Window::FramebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void Window::KeyPressCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if(key == GLFW_KEY_P && action == GLFW_PRESS)
        {
            isCursorDisabled = !isCursorDisabled; 
            if(isCursorDisabled)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
                
            else
            {
                camera.firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
                
        }
    }

    Window::~Window()
    {
        std::cout << "Destruction Window... \n";
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}


