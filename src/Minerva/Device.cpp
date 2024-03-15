#include "Device.h"
#include "DebugManager.h"
#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <iostream>

namespace Minerva
{
    void Device::PickMostSuitableDevice(const VkInstance& vulkanInstance)
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
            int score = RateCurrentDevice(currentdevice);
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

    int Device::RateCurrentDevice(VkPhysicalDevice currentDevice)
    {
        QueueFamilyIndices indices = FindQueueFamilies(currentDevice);
        if (!indices.IsComplete())
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

    QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice currentDevice)
    {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(currentDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(currentDevice, &queueFamilyCount, queueFamilies.data());
        int index = 0;
        for(const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = index;
            }
            index++;
        }
        return indices;
    }
    void Device::CreateLogicalDevice(DebugManager& debugManager)
    {
        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;

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
        vkDestroyDevice(logicalDevice, nullptr);
    }
}