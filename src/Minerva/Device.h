#pragma once
#include "vulkan/vulkan.h"
#include <optional>

namespace Minerva
{
    /// @brief Is a struct which conteins indices of a queue family
    struct QueueFamilyIndices
    {
        /*Is the graphics queue family used to submit rendering operations. I use std::optional 
        to check if the var has already been assigned*/
        std::optional<uint32_t> graphicsFamily;
        bool IsComplete() const
        {
            return graphicsFamily.has_value();
        }
    };
    class DebugManager;
    class Device
    {
    public:
        //The handle of logical device
        VkDevice logicalDevice = VK_NULL_HANDLE;
        /// @brief Pick the best physical device 
        /// @param vulkanInstance The Vulkan instance
        void PickMostSuitableDevice(const VkInstance& vulkanInstance);
        /// @brief Finds a queue family index and used it to create a new QueueFamilyIndices obj.
        /// At the moment I search only for a queue capable to graphics operations
        /// @param currentDevice The current physical device where I search the queue
        /// @return The QueueFamilyIndices obj
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice currentDevice);
        /// @brief Creates the logical device and the graphics queue 
        /// @param debugManager The DebugManager obj useful to access  to validationLayers vector 
        void CreateLogicalDevice(DebugManager& debugManager);
        /// @brief Prints some info about the selected physical device
        void PrintInfoDeviceSelected();
        Device() = default;
        ~Device();
    private: 
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        /// @brief Rates the current device based on some device properties and features
        /// @param currentDevice The physical device that I want to rate
        /// @return The rate 
        int RateCurrentDevice(VkPhysicalDevice currentDevice);
    };
}


