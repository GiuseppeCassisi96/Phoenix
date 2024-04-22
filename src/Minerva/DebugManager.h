#pragma once
#include <vector>
#include <string>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <map>

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
        /// @brief Checks if the validation layer contained in the 'validationLayers' vector is supported. 
        /// @return If founds the validation layer return true otherwise return false 
        bool CheckValidationLayersSupport();
        /// @brief Setups the VkDebugUtilsMessengerCreateInfoEXT struct. It defines the severity and the type 
        ///of the accepted messages, and also it defines the callback function where messages will be printed
        /// @param createInfo An instance of the struct 
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        /// @brief Finds needed extensions to create a surface in Vulkan for GLFW windows
        /// @return The extensions found
        std::vector<const char*> GetRequiredExtensions();
        /// @brief Creates the debug messenger
        /// @param instance The Vulkan instance
        void SetupDebugMessenger(const VkInstance& instance);
        /// @brief Is a utility function which gets the address of the function useful to create the debug messenger 
        /// @param instance The Vulkan instance
        /// @param pCreateInfo The info of the debug messenger
        /// @param pAllocator Is an optional param useful to specify a callback for a memory allocator
        /// @param pDebugMessenger The obj handle for the debug messenger 
        /// @return The result of creation process
        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);
        /// @brief Is a utility function which gets the address of the function useful to destroy the debug messenger
        /// @param instance The Vulkan instance
        /// @param debugMessenger The obj handle for the debug messenger 
        /// @param pAllocator Is an optional param useful to specify a callback for a memory allocator
        void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator
        );
    private:
        

        /// @brief Is the callback function which is going to print the message
        /// @param messageSeverity The severity of a message. It could be an error, a warning
        /// or a simple diagnostic message
        /// @param messageType Represents the type of message. 
        /// @param pCallbackData Are useful callback data about the message (e.g. error info) 
        /// @param pUserData Is a void pointer usually null
        /// @return Returns a bool that indicates if the calling function should be aborted 
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

        
    };
  
}