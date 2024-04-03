#include "Mesh.h"
#include "EngineVars.h"
#include <unordered_map>

#include <iostream>

namespace Minerva
{

    void Transformation::Move(const glm::vec3 &dir, glm::mat4 model)
    {
        ubo.model = glm::translate(model, dir);
    }
    void Transformation::Scale(const glm::vec3& dim, glm::mat4 model)
    {
        ubo.model = glm::scale(model, dim);
    }
    void Transformation::Rotate(const float& angle, const glm::vec3& axis, glm::mat4 model)
    {
        ubo.model = glm::rotate(model, glm::radians(angle), axis);
    }
    

    Mesh::Mesh(std::vector<Vertex> inputVertices, std::vector<uint32_t> inputIndices): vertices(inputVertices), 
    indices(inputIndices)
    {}

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
        // TODO: inserire l'istruzione return qui
    }
}