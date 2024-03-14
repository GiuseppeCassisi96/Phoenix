#include "VulkanInstance.h"
#include <stdexcept>
#include <iostream>

namespace Minerva
{
   

    void VulkanInstance::CreateInstance()
    {
        //Check for validation layers
        if (enableValidationLayers && !debugLayer.CheckValidationLayersSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo VKappInfo {};
        VKappInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        VKappInfo.pApplicationName = "VulkanLearning";
        VKappInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        VKappInfo.pEngineName = "No Engine";
        VKappInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        VKappInfo.apiVersion = VK_API_VERSION_1_0;


        VkInstanceCreateInfo VKinstanceInfo {};
        VKinstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        VKinstanceInfo.pApplicationInfo = &VKappInfo;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) 
        {
            VKinstanceInfo.enabledLayerCount = static_cast<uint32_t>(debugLayer.validationLayers.size());
            VKinstanceInfo.ppEnabledLayerNames = debugLayer.validationLayers.data();
            debugLayer.PopulateDebugMessengerCreateInfo(debugCreateInfo);
            VKinstanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else 
        {
            VKinstanceInfo.enabledLayerCount = 0;
            VKinstanceInfo.pNext = nullptr;
        }
        
        auto recExtensions = debugLayer.GetRequiredExtensions();
        VKinstanceInfo.enabledExtensionCount = static_cast<uint32_t>(recExtensions.size());
        VKinstanceInfo.ppEnabledExtensionNames = recExtensions.data();
        
        VKinstanceInfo.enabledLayerCount = 0;

        VkResult result = vkCreateInstance(&VKinstanceInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }
     VulkanInstance::~VulkanInstance()
    {
        std::cout << "Destruction VulkanInstance... \n";
        vkDestroyInstance(instance, nullptr);
    }
}
