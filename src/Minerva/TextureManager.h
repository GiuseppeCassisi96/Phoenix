#pragma once 
#include "vulkan/vulkan.h"
#include "String"
namespace Minerva
{
    class TextureManager
    {
    public:
        VkImageView textureImageView = VK_NULL_HANDLE;
        VkSampler textureSampler = VK_NULL_HANDLE;
        void CreateTextureImage(std::string textureFileName);  
        void CreateTextureImageView();
        void CreateTextureSampler();
        void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, 
        VkDeviceMemory& imageMemory);
        TextureManager() = default;
        ~TextureManager();
    private:
        const std::string TEXTURES_PATH = "C:/UNIMI/TESI/Phoenix/src/Minerva/Textures/";
        VkImage textureImage = VK_NULL_HANDLE;
        VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
        
    };
}