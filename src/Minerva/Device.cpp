#include "Device.h"
#include "DebugManager.h"
#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <iostream>

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
        vkDestroyDevice(logicalDevice, nullptr);
    }
}