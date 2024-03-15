#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "DebugManager.h"
namespace Minerva
{
    class VulkanInstance
    {
    public:
        VulkanInstance() = default;
        ~VulkanInstance();
        VkInstance instance = nullptr;
        DebugManager debugLayer;
        /// @brief Creates the instance of Vulkan using hardcoded informations 
        void CreateInstance();
    };
}
