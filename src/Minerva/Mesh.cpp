#include "Mesh.h"
#include "EngineVars.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


namespace Minerva
{

    Mesh::~Mesh()
    {
        vkDestroyBuffer(engineDevice.logicalDevice, meshBuffer.indexBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, meshBuffer.indexBufferMemory, nullptr);
        vkDestroyBuffer(engineDevice.logicalDevice, meshBuffer.vertexBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, meshBuffer.vertexBufferMemory, nullptr);
    }

    Mesh::Mesh(Mesh &&other) noexcept
    {
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        meshBuffer.size = std::move(other.meshBuffer.size);
        meshBuffer.vertexBuffer = std::move(other.meshBuffer.vertexBuffer);
        meshBuffer.vertexBufferMemory = std::move(other.meshBuffer.vertexBufferMemory);
        meshBuffer.indexBuffer = std::move(other.meshBuffer.indexBuffer);
        meshBuffer.indexBufferMemory = std::move(other.meshBuffer.indexBufferMemory);

        free(other.vertices.data());
        free(other.indices.data());
        other.meshBuffer.size = 0;
        vkDestroyBuffer(engineDevice.logicalDevice, other.meshBuffer.indexBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, other.meshBuffer.indexBufferMemory, nullptr);
        vkDestroyBuffer(engineDevice.logicalDevice, other.meshBuffer.vertexBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, other.meshBuffer.vertexBufferMemory, nullptr);

    }
    Mesh &Mesh::operator=(Mesh &&other) noexcept
    {
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        meshBuffer.size = std::move(other.meshBuffer.size);
        meshBuffer.vertexBuffer = std::move(other.meshBuffer.vertexBuffer);
        meshBuffer.vertexBufferMemory = std::move(other.meshBuffer.vertexBufferMemory);
        meshBuffer.indexBuffer = std::move(other.meshBuffer.indexBuffer);
        meshBuffer.indexBufferMemory = std::move(other.meshBuffer.indexBufferMemory);

        free(other.vertices.data());
        free(other.indices.data());
        other.meshBuffer.size = 0;
        vkDestroyBuffer(engineDevice.logicalDevice, other.meshBuffer.indexBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, other.meshBuffer.indexBufferMemory, nullptr);
        vkDestroyBuffer(engineDevice.logicalDevice, other.meshBuffer.vertexBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, other.meshBuffer.vertexBufferMemory, nullptr);

        return *this;
    }
}

namespace std {
    template<> struct hash<Minerva::Mesh::Vertex> {
        size_t operator()(Minerva::Mesh::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}