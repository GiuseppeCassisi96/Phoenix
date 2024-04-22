#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "DebugManager.h"
namespace Minerva
{
    class VulkanInstance
    {
    public:
        
        VkInstance instance = nullptr;
        /// @brief Creates the instance of Vulkan using hardcoded informations 
        void CreateInstance();

        VulkanInstance() = default;
        ~VulkanInstance();

        VulkanInstance(const VulkanInstance& other) = delete;
        VulkanInstance& operator=(const VulkanInstance& other) = delete;

        VulkanInstance(VulkanInstance&& other) noexcept;
        VulkanInstance& operator=(VulkanInstance&& other) noexcept;
    };
}
