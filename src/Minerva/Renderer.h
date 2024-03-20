#pragma once
#include "vulkan/vulkan.h"
#include "vector"
namespace Minerva
{
    class Renderer
    {
    public:
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        bool framebufferResized = false;
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateCommandPool();
        void CreateCommandBuffer();
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void DrawFrame();
        void CreateSyncObjects();
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        Renderer() = default;
        ~Renderer();
    private:
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;;
        VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;;
        const int MAX_FRAMES_IN_FLIGHT = 2;
        uint32_t currentFrame = 0;
        
    };
    
    
}