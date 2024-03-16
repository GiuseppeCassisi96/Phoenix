#pragma once
#include "vulkan/vulkan.h"
#include <optional>
#include <vector>

namespace Minerva
{
    /// @brief Is a struct which conteins indices of a queue family
    struct QueueFamilyIndices
    {
        /*Is the graphics queue family used to submit rendering operations. I use std::optional 
        to check if the var has already been assigned*/
        std::optional<uint32_t> graphicsFamily;
        /*Is a presentation queue which presents images to the surface created in Window class*/
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails 
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class DebugManager;
    class Device
    {
    public:
        //The handle of logical device
        VkDevice logicalDevice = VK_NULL_HANDLE;
        /// @brief Pick the best physical device 
        /// @param vulkanInstance The Vulkan instance
        void PickMostSuitableDevice(const VkInstance& vulkanInstance, const VkSurfaceKHR& windowSurface);
        /// @brief Finds a queue families indices and used them to create a new QueueFamilyIndices obj.
        /// At the moment I search only for a queues capable to graphics operations and presentation operations
        /// @param currentDevice The current physical device where I search the queue
        /// @param windowSurface The surface where I search the support
        /// @return The QueueFamilyIndices obj
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice currentDevice, const VkSurfaceKHR& windowSurface);
        /// @brief Creates the logical device and the graphics queue 
        /// @param debugManager The DebugManager obj useful to access  to validationLayers vector 
        void CreateLogicalDevice(DebugManager& debugManager, const VkSurfaceKHR& windowSurface);
        /// @brief Prints some info about the selected physical device
        void PrintInfoDeviceSelected();
        Device() = default;
        ~Device();
    private: 
        const std::vector<const char*> neededDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentationQueue = VK_NULL_HANDLE;
        /// @brief Rates the current device based on some device properties and features
        /// @param currentDevice The physical device that I want to rate
        /// @return The rate 
        int RateCurrentDevice(VkPhysicalDevice currentDevice, const VkSurfaceKHR& windowSurface);
        bool CheckDeviceExtensionsSupport(VkPhysicalDevice currentDevice);
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice currentDevice);
    };
}


