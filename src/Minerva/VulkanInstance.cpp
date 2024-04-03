#include "VulkanInstance.h"
#include <stdexcept>
#include <iostream>
#include "EngineVars.h"

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
        VKappInfo.pApplicationName = "Minerva";
        VKappInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        VKappInfo.pEngineName = "Minerva";
        VKappInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        VKappInfo.apiVersion = VK_API_VERSION_1_3;


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
        

        VkResult result = vkCreateInstance(&VKinstanceInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }
     VulkanInstance::~VulkanInstance()
    {
        std::cout << "Destruction Vulkan instance... \n";
        vkDestroyInstance(instance, nullptr);
    }
    VulkanInstance::VulkanInstance(VulkanInstance &&other) noexcept
    {
        instance = other.instance;
        vkDestroyInstance(other.instance, nullptr);
    }
    VulkanInstance &VulkanInstance::operator=(VulkanInstance &&other) noexcept
    {
        instance = other.instance;
        vkDestroyInstance(other.instance, nullptr);
        
        return *this;
    }
}
