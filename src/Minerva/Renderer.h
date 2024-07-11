#pragma once
#include "vulkan/vulkan.h"
#include "vector"
#include "Mesh.h"
#include "Phoenix/PhoenixMesh.h"
#include "Phoenix/LODSelectionDispatcher.h"


namespace Minerva
{
    struct UniformBuffers
    {
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        std::vector<void*> uniformBuffersMapped;
    };

    enum Mode
    {
        Phoenix = 0,
        Normal = 1
    };

    struct BoneMatricesUniformType
    {
        glm::mat4 finalBoneMatrices[MAX_BONES];
    };
    class Renderer
    {
    public:

        struct IndirectCommandsBuffer
        {
            VkBuffer buffer{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
        };
        
        Mode renderMode;
        std::vector<VkBuffer> InputSSBO;
        std::vector<VkDeviceMemory> InputMemorySSBO;

        IndirectCommandsBuffer indirectCommandsBuffer;
        std::vector<VkDrawIndexedIndirectCommand> indirectCommands;
        uint32_t currentFrame = 0;
        uint32_t currentComputeFrame = 0;
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool; 
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkCommandBuffer> computeCommandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        std::vector<VkFence> computeInFlightFences;
        std::vector<VkSemaphore> computeFinishedSemaphores;

        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        bool framebufferResized = false;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        const int MAX_FRAMES_IN_FLIGHT = 2;
        UniformBuffers transformationUBuffers; 
        UniformBuffers animUBuffers; 
        BoneMatricesUniformType UNBoneMatrices;
        VkImage colorImage;
        VkDeviceMemory colorImageMemory;
        VkImageView colorImageView;
        std::vector<uint32_t> offsetsVec;

        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateCommandPool();
        void CreateCommandBuffer();
        void CreateComputeCommandBuffer();
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void DrawFrame();
        void CreateSyncObjects();
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, size_t destOffset = 0);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void CreateVertexBuffer();
        void CreateInstanceBuffer();
        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSets(int numberOfMeshlets);
        void UpdateUniformBuffer(uint32_t currentImage, int numberOfMeshlet = 0, glm::vec3 instancePos = glm::vec3{0.0f});
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        void TransitionImageLayout(VkImage image, VkFormat format, 
        VkImageLayout oldLayout, VkImageLayout newLayout);
        void CreateDepthResources();
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, 
        VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat FindDepthFormat();
        void PrepareIndirectData(std::vector<uint32_t>& indexBuffer, std::vector<Mesh::Vertex>
        vertexBuffer);
        void UpdateIndirectData();
        bool HasStencilComponent(VkFormat format);
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void CreateColorResources();
        void UpdateIndexBuffer();
        void UpdateVertexBuffer();
        void CreateIndexBuffer();
        void DispatchCompute(int numberOfMeshlets, glm::vec3 instancePos, void* data);
        void RecordComputeBuffer(VkCommandBuffer commandBuffer, int numberOfMeshlets);

        Renderer() = default;
        ~Renderer();

        Renderer(const Renderer& other) = delete;
        Renderer& operator=(const Renderer& other) = delete;

        Renderer(Renderer&& other) noexcept;
        Renderer& operator=(Renderer&& other) noexcept;

    private:
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
        std::vector<VkDescriptorSet> descriptorSets;
        
    };
    
    
}