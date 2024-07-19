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
        size_t size = totalMeshlets.size() * Minerva::engineModLoader.instanceNumber;
        //Input setting
        inputData.resize(totalMeshlets.size() * Minerva::engineModLoader.instanceNumber);
        constantData.resize(Minerva::engineModLoader.instanceNumber);
        outputData.resize(totalMeshlets.size() * Minerva::engineModLoader.instanceNumber);
        for(int j = 0; j < constantData.size(); j++)
        {
            constantData[j].instancesPos = Minerva::engineModLoader.instancesData[j].instancePos;
            constantData[j].numberOfMeshlet = totalMeshlets.size();
            constantData[j].numberOfInstances = Minerva::engineModLoader.instanceNumber;
            constantData[j].errorThreshold = errorThreshold;
            constantData[j].hfov = hFOV;
            constantData[j].width = width;
            for(int i = j * totalMeshlets.size(); i < totalMeshlets.size() + 
            (j * totalMeshlets.size()); i++)
            {
                PhoenixMeshlet currentMeshlet = totalMeshlets[i - (j * totalMeshlets.size())];
                inputData[i].boundCenter = currentMeshlet.bound.center;
                inputData[i].error = currentMeshlet.error;
                inputData[i].lod = currentMeshlet.lod;
                inputData[i].meshletID = currentMeshlet.meshletID;
                inputData[i].parentBoundCenter = currentMeshlet.parentBound.center;
                inputData[i].parentError = currentMeshlet.parentError;
                inputData[i].numberOfInstance = j;
            }
        }
        outputData[0].index = 0;
        size_t currentSize = 0;
        while(currentSize <= size)
        {
            RENDERER.computeWorkgroup++;
            currentSize += 32;
        }
        RENDERER.InputSSBO.resize(RENDERER.MAX_FRAMES_IN_FLIGHT);
        RENDERER.InputMemorySSBO.resize(RENDERER.MAX_FRAMES_IN_FLIGHT);

        VkDeviceSize bufferSize = (sizeof(InputComputeData) * totalMeshlets.size()) 
        * Minerva::engineModLoader.instanceNumber;

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
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            RENDERER.InputSSBO[i], RENDERER.InputMemorySSBO[i]);

            RENDERER.CopyBuffer(stagingBuffer, RENDERER.InputSSBO[i], bufferSize);
        }

        vkDestroyBuffer(Minerva::engineDevice.logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(Minerva::engineDevice.logicalDevice, stagingBufferMemory, nullptr);

        //----------------------CONSTANT DATA-----------------------------
        RENDERER.ConstantSSBO.resize(RENDERER.MAX_FRAMES_IN_FLIGHT);
        RENDERER.ConstantMemorySSBO.resize(RENDERER.MAX_FRAMES_IN_FLIGHT);

        VkDeviceSize constBufferSize = sizeof(ConstantData) * constantData.size();

        VkBuffer constStagingBuffer;
        VkDeviceMemory constStagingBufferMemory;
        RENDERER.CreateBuffer(constBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, constStagingBuffer, 
        constStagingBufferMemory);

        void* constData;
        vkMapMemory(Minerva::engineDevice.logicalDevice, constStagingBufferMemory, 0, constBufferSize, 0, &constData);
        memcpy(constData, constantData.data(), (size_t)constBufferSize);
        vkUnmapMemory(Minerva::engineDevice.logicalDevice, constStagingBufferMemory);

        for (size_t i = 0; i < RENDERER.MAX_FRAMES_IN_FLIGHT; i++) 
        {
            RENDERER.CreateBuffer(constBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT  
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            RENDERER.ConstantSSBO[i], RENDERER.ConstantMemorySSBO[i]);
            
            RENDERER.CopyBuffer(constStagingBuffer, RENDERER.ConstantSSBO[i], constBufferSize);
        }

        vkDestroyBuffer(Minerva::engineDevice.logicalDevice, constStagingBuffer, nullptr);
        vkFreeMemory(Minerva::engineDevice.logicalDevice, constStagingBufferMemory, nullptr);

        //----------------------OUTPUT DATA-----------------------------
        RENDERER.OutputSSBO.resize(RENDERER.MAX_FRAMES_IN_FLIGHT);
        RENDERER.OutputMemorySSBO.resize(RENDERER.MAX_FRAMES_IN_FLIGHT);
        RENDERER.OutputMappedSSBO.resize(RENDERER.MAX_FRAMES_IN_FLIGHT);

        VkDeviceSize outBufferSize = sizeof(OutputData) * outputData.size();

        VkBuffer outStagingBuffer;
        VkDeviceMemory outStagingBufferMemory;
        RENDERER.CreateBuffer(outBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, outStagingBuffer, 
        outStagingBufferMemory);

        void* outData;
        vkMapMemory(Minerva::engineDevice.logicalDevice, outStagingBufferMemory, 0, outBufferSize, 0, &outData);
        memcpy(outData, outputData.data(), (size_t)outBufferSize);
        vkUnmapMemory(Minerva::engineDevice.logicalDevice, outStagingBufferMemory);

        for (size_t i = 0; i < RENDERER.MAX_FRAMES_IN_FLIGHT; i++) 
        {
            RENDERER.CreateBuffer(outBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT  
            | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            RENDERER.OutputSSBO[i], RENDERER.OutputMemorySSBO[i]);
            
            RENDERER.CopyBuffer(outStagingBuffer, RENDERER.OutputSSBO[i], outBufferSize);
            vkMapMemory(Minerva::engineDevice.logicalDevice, RENDERER.OutputMemorySSBO[i], 0, 
            outBufferSize, 0, &RENDERER.OutputMappedSSBO[i]);
        }

        vkDestroyBuffer(Minerva::engineDevice.logicalDevice, outStagingBuffer, nullptr);
        vkFreeMemory(Minerva::engineDevice.logicalDevice, outStagingBufferMemory, nullptr);

    }
}