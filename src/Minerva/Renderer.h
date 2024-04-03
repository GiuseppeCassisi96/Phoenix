#pragma once
#include "vulkan/vulkan.h"
#include "vector"
#include "Mesh.h"

namespace Minerva
{
    class Renderer
    {
    public:
        uint32_t currentFrame = 0;
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        bool framebufferResized = false;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        const int MAX_FRAMES_IN_FLIGHT = 2;


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
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void CreateVertexBuffer();
        void CreateInstanceBuffer();
        void CreateIndexBuffer();
        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSets();
        void CreateUniformBuffers();
        void UpdateUniformBuffer(uint32_t currentImage);
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        void TransitionImageLayout(VkImage image, VkFormat format, 
        VkImageLayout oldLayout, VkImageLayout newLayout);
        void CreateDepthResources();
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, 
        VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat FindDepthFormat();
        bool HasStencilComponent(VkFormat format);
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        Renderer() = default;
        ~Renderer();

        Renderer(const Renderer& other) = delete;
        Renderer& operator=(const Renderer& other) = delete;

        Renderer(Renderer&& other) noexcept;
        Renderer& operator=(Renderer&& other) noexcept;

    private:

        struct UniformBuffers
        {
            std::vector<VkBuffer> uniformBuffers;
            std::vector<VkDeviceMemory> uniformBuffersMemory;
            std::vector<void*> uniformBuffersMapped;
        };
        UniformBuffers UBuffers;   
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
        std::vector<VkDescriptorSet> descriptorSets;
        
        
    };
    
    
}