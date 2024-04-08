#pragma once
#include <array>
#include "vector"
#include "vulkan/vulkan.h"
#include "string"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define MAX_BONE_PER_VERTEX 2
#define MAX_BONES 100

namespace Minerva
{
    struct InstanceData
    {
        glm::vec3 instancePos;
        float instanceScale;
    };

    class Mesh
    {
    public:
        struct Vertex 
        {
            glm::vec3 offsetPos;
            float offsetScale;
            glm::vec3 pos;
            glm::vec3 color;
            glm::vec2 texCoord;
            int boneID[MAX_BONE_PER_VERTEX];
            float weight[MAX_BONE_PER_VERTEX];
            

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

            static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions() 
            {
                std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};
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

                attributeDescriptions[5].binding = 0;
                attributeDescriptions[5].location = 5;
                attributeDescriptions[5].format = VK_FORMAT_R32G32_SINT;
                attributeDescriptions[5].offset = offsetof(Vertex, boneID);

                attributeDescriptions[6].binding = 0;
                attributeDescriptions[6].location = 6;
                attributeDescriptions[6].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[6].offset = offsetof(Vertex, weight);
                return attributeDescriptions;
            }
        };
        enum MeshType
        {
            Static = 0,
            Skeletal = 1
        };
        struct InstanceBuffer 
        {
            VkBuffer buffer{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
            size_t size = 0;
        } ;

         struct MeshBuffer
        {
            VkBuffer vertexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
            VkBuffer indexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
            size_t size = 0;
        };

        struct BoneInfo
        {
            int id;
            glm::mat4 offset;
        };
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        MeshBuffer meshBuffer;
        MeshType typeOfMesh;

        Mesh() = default;
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
    private:
        const std::string MODELS_PATH = "C:/UNIMI/TESI/Phoenix/src/Minerva/Models/";
    };
    
}


