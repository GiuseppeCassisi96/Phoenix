#pragma once
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

//NDEBUG means 'not debug'
 #ifdef NDEBUG
     const bool enableValidationLayers = false;
 #else
     const bool enableValidationLayers = true;
 #endif 
namespace Minerva
{
    class DebugManager
    { 
    public:
        VkDebugUtilsMessengerEXT debugMessenger;
        const std::vector<const char*> validationLayers ={ "VK_LAYER_KHRONOS_validation"};
        bool CheckValidationLayersSupport();
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        std::vector<const char*> GetRequiredExtensions();
        void SetupDebugMessenger(const VkInstance& instance);
        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);
        void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator
        );
        
    
    private:

        //VkInstance engineInstance;
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

        
    };
  
}