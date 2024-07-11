#include "LODSelectionDispatcher.h"
#include <iostream>
#include "Minerva/EngineVars.h"
#define RENDERER Minerva::engineRenderer
namespace Phoenix
{
    void LODSelectionDispatcher::PrepareComputeData(const std::vector<PhoenixMeshlet>&  totalMeshlets,
    float hFOV, int width)
    {
        meshletForSelection = totalMeshlets;

        //Input setting
        inputData.resize(totalMeshlets.size());
        for(int i = 0; i < totalMeshlets.size(); i++)
        {
            PhoenixMeshlet currentMeshlet = totalMeshlets[i];
            inputData[i].boundCenter = currentMeshlet.bound.center;
            inputData[i].error = currentMeshlet.error;
            inputData[i].errorThreshold = errorThreshold;
            inputData[i].hfov = hFOV;
            inputData[i].lod = currentMeshlet.lod;
            inputData[i].meshletID = currentMeshlet.meshletID;
            inputData[i].parentBoundCenter = currentMeshlet.parentBound.center;
            inputData[i].parentError = currentMeshlet.parentError;
            inputData[i].width = width;
            inputData[i].isSelected = false;
        }

        RENDERER.InputSSBO.resize(RENDERER.MAX_FRAMES_IN_FLIGHT);
        RENDERER.InputMemorySSBO.resize(RENDERER.MAX_FRAMES_IN_FLIGHT);

        VkDeviceSize bufferSize = sizeof(InputComputeData) * totalMeshlets.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        RENDERER.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(Minerva::engineDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, inputData.data(), (size_t)bufferSize);
        vkUnmapMemory(Minerva::engineDevice.logicalDevice, stagingBufferMemory);

        for (size_t i = 0; i < RENDERER.MAX_FRAMES_IN_FLIGHT; i++) 
        {
            RENDERER.CreateBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT  
            | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, RENDERER.InputSSBO[i], 
            RENDERER.InputMemorySSBO[i]);
            
            RENDERER.CopyBuffer(stagingBuffer, RENDERER.InputSSBO[i], bufferSize);
        }

        vkDestroyBuffer(Minerva::engineDevice.logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(Minerva::engineDevice.logicalDevice, stagingBufferMemory, nullptr);

    }

    std::vector<uint32_t> LODSelectionDispatcher::LodSelector(int width, float hFov, const glm::vec3& instancePos, 
    std::vector<MINERVA_VERTEX>& vertexBuffer, Minerva::Transformation& tr, PhoenixMesh& mesh, int& vertexCount)
    {
        std::vector<uint32_t> newIndexBuffer;  
        std::unordered_set<idx_t> meshletsSelected;
        
        for(const auto& meshlet : meshletForSelection) 
        {
            glm::mat4 groupMatrix = tr.ubo.view * tr.ubo.model;
            float currentError = ComputeScreenSpaceError(meshlet.bound, meshlet.error, width,
            hFov, instancePos, groupMatrix);

            if(meshlet.lod >= MAX_LOD_NUMBER - 1)
            {
                if(currentError <= errorThreshold)
                {
                    meshletsSelected.insert(meshlet.meshletID);
                }  
                continue;
            } 

            glm::mat4 parentGroupMatrix = tr.ubo.view * tr.ubo.model;
            float parentError = ComputeScreenSpaceError(meshlet.parentBound, meshlet.parentError,
            width, hFov, instancePos, parentGroupMatrix);

            if(currentError <= errorThreshold && parentError > errorThreshold)
            {
                meshletsSelected.insert(meshlet.meshletID);
            }
                          
        } 
        int count = 0;
        for(const auto& meshletID : meshletsSelected)
        {
            PhoenixMeshlet currentPMeshlet = meshletForSelection[meshletID];    
            
            newIndexBuffer.insert(newIndexBuffer.end(), currentPMeshlet.meshletIndexBuffer.begin(),
            currentPMeshlet.meshletIndexBuffer.end()); 

            count += currentPMeshlet.vertexCount;

            mesh.ColourGroups(currentPMeshlet, vertexBuffer);
        }
        vertexCount = count;

        return newIndexBuffer;
    }

    float LODSelectionDispatcher::ComputeScreenSpaceError(PhoenixBound bound,float groupError,int width, 
    float hFov, const glm::vec3& instancePos, const glm::mat4& modelView)
    {
        bound.center += instancePos;
        glm::vec4 viewCenter = glm::vec4(bound.center, 1.0f);
        //I transform the center in view-space
        viewCenter = modelView * viewCenter;
        bound.center.x = viewCenter.x;
        bound.center.y = viewCenter.y;
        bound.center.z = viewCenter.z;
        bound.radius = glm::length(glm::vec3 {modelView * glm::vec4{groupError, 0, 0, 0 }});
  
        //https://stackoverflow.com/questions/21648630/radius-of-projected-sphere-in-screen-space
        const float cotHalfFov = 1.0f / glm::tan(hFov / 2.0f);
        const float d2 = glm::dot(bound.center, bound.center);
        const float r = bound.radius;
        const float div = glm::sqrt(d2 - r*r);
        float screenSpaceError = (width / 2.0f * cotHalfFov * r) / div;
        return screenSpaceError;

    }
}