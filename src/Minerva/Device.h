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
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
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
        /// @brief Obtains all swap chain datails of current device. 
        /// The swap chain details are the surface formats and surface presentation modes  
        /// @param currentDevice Is a physical device where I want to query swap chain details
        /// @param windowSurface The window surface needed to get formats and presntation modes
        /// @return 
        SwapChainSupportDetails QuerySwapChainDetails(VkPhysicalDevice currentDevice,
        const VkSurfaceKHR& windowSurface);
        /// @brief Creates the swap chain based on surface format, surface presentation mode and surface extent
        void CreateSwapChain();
        void CreateImageViews();
        Device() = default;
        ~Device();
    private: 
        const std::vector<const char*> neededDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentationQueue = VK_NULL_HANDLE;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        /// @brief Rates the current device based on some device properties and features
        /// @param currentDevice The physical device that I want to rate
        /// @return The rate 
        int RateCurrentDevice(VkPhysicalDevice currentDevice, const VkSurfaceKHR& windowSurface);
        /// @brief Checks if the current device has the support the needed device extensions
        /// @param currentDevice The ckecked device 
        /// @return True if the device has the support false otherwise
        bool CheckDeviceExtensionsSupport(VkPhysicalDevice currentDevice);
        /// @brief Chooses the surface format amagon all available surface formats based on a 
        ///choosen format and color space
        /// @param availableFormats All available surface formats
        /// @param desiredFormat The desired format
        /// @param desiredColorSpace The desired color space
        /// @return The choosen surface format
        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats,
        const VkFormat& desiredFormat, const VkColorSpaceKHR& desiredColorSpace);
        /// @brief Chooses the surface presentation mode amagon all available surface presentation modes.
        /// @param availablePresentModes 
        /// @return The desired presentation mode if doesn't found The desired presentation return 
        /// VK_PRESENT_MODE_FIFO_KHR presentation mode
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes,
        const VkPresentModeKHR& desiredPresentMode);
        /// @brief Defines the swap chain extent based on framebuffer size
        /// @param capabilities Capabilities of window surface
        /// @return The extent of swap chain 
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
    };
}


