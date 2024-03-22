#include "Device.h"
#include "DebugManager.h"
#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include "EngineVars.h"
#include <algorithm> 

namespace Minerva
{
    void Device::PickMostSuitableDevice(const VkInstance& vulkanInstance, const VkSurfaceKHR& windowSurface)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

        std::multimap<int, VkPhysicalDevice> devicesByScore;
        for (auto currentdevice : devices)
        {
            int score = RateCurrentDevice(currentdevice, windowSurface);
            devicesByScore.insert(std::make_pair(score, currentdevice));
        }
        if (devicesByScore.rbegin()->first > 0)
        {
            physicalDevice = devicesByScore.rbegin()->second;
        }
        else
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    int Device::RateCurrentDevice(VkPhysicalDevice currentDevice, const VkSurfaceKHR& windowSurface)
    {
        QueueFamilyIndices indices = FindQueueFamilies(currentDevice, windowSurface);
        if (!indices.IsComplete())
        {
            return 0;
        }

        /*Checks if the current device has the swap chain extension support. I want a GPU which
        has the swap chain extension*/
        bool extensionsSupported = CheckDeviceExtensionsSupport(currentDevice);
        if (!extensionsSupported)
        {
            return 0;
        }


        SwapChainSupportDetails SCSDetails = QuerySwapChainDetails(currentDevice, windowSurface);
        if(SCSDetails.formats.empty() || SCSDetails.presentModes.empty())
        {
            return 0;
        }

        int score = 0;
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(currentDevice, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(currentDevice, &deviceFeatures);
       

        //I want only a dedicated GPU
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 200;
        }else{
            return  0;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;
        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader)
        {
            return 0;
        }

        //The tessellation shaders is a plus
        if (deviceFeatures.tessellationShader)
        {
            score += 100;
        }
        return score;
    }

    bool Device::CheckDeviceExtensionsSupport(VkPhysicalDevice currentDevice)
    {

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(currentDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(currentDevice, nullptr, &extensionCount, availableExtensions.data());
        //I create a util set which stores all required extensions
        std::set<std::string> requiredExtensions(neededDeviceExtensions.begin(), neededDeviceExtensions.end());

        for (const auto& availableExtension : availableExtensions)
        {
            //I delete from the set the name equal to availableExtension.extensionName
            requiredExtensions.erase(availableExtension.extensionName);
        }
        //if requiresExtensions is empty it means that all required extensions are supported
        return requiredExtensions.empty();
    }

    SwapChainSupportDetails Device::QuerySwapChainDetails(VkPhysicalDevice currentDevice, const VkSurfaceKHR &windowSurface)
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(currentDevice, windowSurface, &details.capabilities);

        /*I get surface formats count if the count is different from zero 
        I fill the details.format array*/
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(currentDevice, windowSurface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(currentDevice, windowSurface, &formatCount,
            details.formats.data());
        }

        /*I get surface presentation modes count if the count is different from zero 
        I fill the details.presentModes array*/
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(currentDevice,windowSurface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(currentDevice, windowSurface, &presentModeCount,
             details.presentModes.data());
        }
        return details;
    }

    QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice currentDevice, const VkSurfaceKHR& windowSurface)
    {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(currentDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(currentDevice, &queueFamilyCount, queueFamilies.data());
        int index = 0;
        for(const auto& queueFamily : queueFamilies)
        {
            VkBool32 presentSupport = false;
            //Checks if a queue family has the capability of presenting to our window surface
            vkGetPhysicalDeviceSurfaceSupportKHR(currentDevice, index, windowSurface, &presentSupport);
            //Then simply check the value of the boolean and store the presentation family queue index
            if (presentSupport)
            {
                indices.presentFamily = index;
            }
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = index;
            }
            index++;
        }
        return indices;
    }
    void Device::CreateLogicalDevice(DebugManager& debugManager, const VkSurfaceKHR& windowSurface)
    {
        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, windowSurface);
        std::vector<VkDeviceQueueCreateInfo> queuesInfo {};
        std::set<uint32_t> engineFamilies {indices.graphicsFamily.value(), indices.presentFamily.value()};
        float queuePriority = 1.0f;
        for(auto engineFamily : engineFamilies)
        {
            VkDeviceQueueCreateInfo tempQueueCreateInfo{};
            tempQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            tempQueueCreateInfo.queueFamilyIndex = engineFamily;
            tempQueueCreateInfo.queueCount = 1;
            tempQueueCreateInfo.pQueuePriorities = &queuePriority;
            queuesInfo.emplace_back(tempQueueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queuesInfo.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queuesInfo.size());
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(neededDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = neededDeviceExtensions.data();

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(debugManager.validationLayers.size());
            createInfo.ppEnabledLayerNames = debugManager.validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }
        vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentationQueue);
    }

    void Device::PrintInfoDeviceSelected()
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        std::cout << " ___________Current device selected by Minerva____________\n";
        std::cout << "|                                                         |\n";
        std::cout << "|   Device name: " << deviceProperties.deviceName << "    |\n";
        std::cout << "|   Driver: " << deviceProperties.driverVersion << "                                    |\n";
        std::cout << "|   Vendor ID: " << deviceProperties.vendorID << "                                       |\n";
        std::cout << "|_________________________________________________________|\n";
        std::cout << "\n\n";
    }

    Device::~Device()
    {
        std::cout << "Destruction Device... \n";
         for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(logicalDevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
        vkDestroySurfaceKHR(engineInstance.instance, windowInstance.windowSurface, nullptr);
        vkDestroyDevice(logicalDevice, nullptr);
        
    }

    void Device::CleanupSwapChain()
    {
        for (auto framebuffer : engineRenderer.swapChainFramebuffers) {
            vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(logicalDevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
    }

    VkImageView Device::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }

        return imageView;
    }

    void Device::RecreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(windowInstance.window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(windowInstance.window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(logicalDevice);

        CleanupSwapChain();

        CreateSwapChain();
        CreateImageViews();
        engineRenderer.CreateDepthResources();
        engineRenderer.CreateFramebuffers();
    }

    VkSurfaceFormatKHR Device::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats,
    const VkFormat& desiredFormat, const VkColorSpaceKHR& desiredColorSpace)
    {
        for(const auto& availableFormat : availableFormats)
        {
            if(availableFormat.format == desiredFormat && 
            availableFormat.colorSpace == desiredColorSpace)
            {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR Device::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes, 
    const VkPresentModeKHR& desiredPresentMode)
    {
        for (const auto& availablePresentMode : availablePresentModes) {
                if (availablePresentMode == desiredPresentMode) {
                    return availablePresentMode;
                }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Device::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        int width, height;
        glfwGetFramebufferSize(windowInstance.window, &width, &height);
        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
        
    }

    void Device::CreateSwapChain()
    {
        //I check if my physical device supports the swap chain
        SwapChainSupportDetails swapChainSupport = QuerySwapChainDetails(physicalDevice, windowInstance.windowSurface);

        VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(swapChainSupport.formats,
        VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes, VK_PRESENT_MODE_MAILBOX_KHR);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
        uint32_t imageNumber = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageNumber > swapChainSupport.capabilities.maxImageCount) {
            imageNumber = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = windowInstance.windowSurface;
        createInfo.minImageCount = imageNumber;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, windowInstance.windowSurface);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageNumber, nullptr);
        swapChainImages.resize(imageNumber);
        vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageNumber, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }
    void Device::CreateImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }
}