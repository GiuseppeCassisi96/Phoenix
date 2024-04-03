#pragma once
#include <glm/glm.hpp>
#include <array>
#include "vector"
#include "vulkan/vulkan.h"
#include "string"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Minerva
{
    struct InstanceData
    {
        glm::vec3 instancePos;
        float instanceScale;
    };
    struct Vertex 
    {
        glm::vec3 offsetPos;
        float offsetScale;
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;
        

        bool operator==(const Vertex& other) const {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }

        static  std::array<VkVertexInputBindingDescription, 2> getBindingDescription() 
        {
            std::array<VkVertexInputBindingDescription, 2> bindingDescriptions;
            bindingDescriptions[0].binding = 0;
            bindingDescriptions[0].stride = sizeof(Vertex);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            bindingDescriptions[1].binding = 1;
            bindingDescriptions[1].stride = sizeof(InstanceData);
            bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            return bindingDescriptions;
        }

        static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() 
        {
            std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
            //Position
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            //Color
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            //UV coord
            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            //Instance pos
            attributeDescriptions[3].binding = 1;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, offsetPos);

            attributeDescriptions[4].binding = 1;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = VK_FORMAT_R32_SFLOAT;
            attributeDescriptions[4].offset = offsetof(Vertex, offsetScale);
            return attributeDescriptions;
        }
    };

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;

    };


    class Transformation
    {
        public:
            UniformBufferObject ubo {};
            void Move(const glm::vec3& dir, glm::mat4 model = glm::mat4(1.0f));
            void Scale(const glm::vec3& dim, glm::mat4 model = glm::mat4(1.0f));
            void Rotate(const float& angle, const glm::vec3& axis, glm::mat4 model = glm::mat4(1.0f));
    };

    class Mesh
    {
    public:

        struct InstanceBuffer 
        {
            VkBuffer buffer{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
            size_t size = 0;
        } ;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        struct MeshBuffer
        {
            VkBuffer vertexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
            VkBuffer indexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
            size_t size = 0;
        };
        MeshBuffer meshBuffer;

        Mesh() = default;
        Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices); 
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
    private:
        const std::string MODELS_PATH = "C:/UNIMI/TESI/Phoenix/src/Minerva/Models/";
    };
    
}


namespace std {
    template<> struct hash<Minerva::Vertex> {
        size_t operator()(Minerva::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}