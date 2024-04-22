#include "Renderer.h"
#include <stdexcept>
#include <iostream>
#include <chrono>
#include "EngineVars.h"




namespace Minerva
{
    void Renderer::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = engineDevice.swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT 
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(engineDevice.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
    void Renderer::CreateFramebuffers()
    {
        swapChainFramebuffers.resize(engineDevice.swapChainImageViews.size());
        for (size_t i = 0; i < engineDevice.swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                engineDevice.swapChainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = engineDevice.swapChainExtent.width;
            framebufferInfo.height = engineDevice.swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(engineDevice.logicalDevice, &framebufferInfo, nullptr, 
            &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }
    void Renderer::CreateCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = engineDevice.FindQueueFamilies(engineDevice.physicalDevice, 
        windowInstance.windowSurface);
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        if (vkCreateCommandPool(engineDevice.logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    void Renderer::CreateCommandBuffer()
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(engineDevice.logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = engineDevice.swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.17f, 0.3f, 0.75f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);         
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, enginePipeline.graphicsPipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(engineDevice.swapChainExtent.width);
            viewport.height = static_cast<float>(engineDevice.swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = engineDevice.swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            Mesh* mesh = &engineModLoader.sceneMeshes[0];
            VkBuffer vertexBuffers[] = {mesh->meshBuffer.vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindVertexBuffers(commandBuffer, 1, 1, &engineModLoader.instanceBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, mesh->meshBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
            enginePipeline.pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
            
            
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->indices.size()), 
            engineModLoader.instanceNumber, 0, 0, 0);

            engineUI.RenderUI(commandBuffers[currentFrame]);

        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
    void Renderer::DrawFrame()
    {
        vkWaitForFences(engineDevice.logicalDevice, 1, 
        &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(engineDevice.logicalDevice, engineDevice.swapChain,
         UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            engineDevice.RecreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        
        UpdateUniformBuffer(currentFrame);
        vkResetFences(engineDevice.logicalDevice, 1, &inFlightFences[currentFrame]);
        vkResetCommandBuffer(commandBuffers[currentFrame],  0);
        RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);
        
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(engineDevice.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {engineDevice.swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(engineDevice.presentationQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            engineDevice.RecreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    
    }

    void Renderer::CreateSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(engineDevice.logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) 
            != VK_SUCCESS ||
                vkCreateSemaphore(engineDevice.logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) 
                != VK_SUCCESS ||
                vkCreateFence(engineDevice.logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void Renderer::CreateVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(engineModLoader.sceneMeshes[0].vertices[0]) * 
        engineModLoader.sceneMeshes[0].vertices.size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
         | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(engineDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, engineModLoader.sceneMeshes[0].vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(engineDevice.logicalDevice, stagingBufferMemory);

        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, engineModLoader.sceneMeshes[0].meshBuffer.vertexBuffer, 
        engineModLoader.sceneMeshes[0].meshBuffer.vertexBufferMemory);
        CopyBuffer(stagingBuffer, engineModLoader.sceneMeshes[0].meshBuffer.vertexBuffer, bufferSize);

        //destroy the staging buffer
        vkDestroyBuffer(engineDevice.logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, stagingBufferMemory, nullptr);
    }

    void Renderer::CreateInstanceBuffer()
    {
        VkDeviceSize bufferSize = engineModLoader.instanceBuffer.size;
        //Temp buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
         | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        if(vkMapMemory(engineDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data)!= VK_SUCCESS)
        {
            throw std::runtime_error("failed to map memory!");
        }
        
        memcpy(data, engineModLoader.instancesData.data(), (size_t) bufferSize);
        vkUnmapMemory(engineDevice.logicalDevice, stagingBufferMemory);
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, engineModLoader.instanceBuffer.buffer, engineModLoader.instanceBuffer.memory);
        CopyBuffer(stagingBuffer, engineModLoader.instanceBuffer.buffer, bufferSize);

        //destroy the staging buffer
        vkDestroyBuffer(engineDevice.logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, stagingBufferMemory, nullptr);
    }

    void Renderer::CreateIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(engineModLoader.sceneMeshes[0].indices[0]) * 
        engineModLoader.sceneMeshes[0].indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(engineDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, engineModLoader.sceneMeshes[0].indices.data(), (size_t) bufferSize);
        vkUnmapMemory(engineDevice.logicalDevice, stagingBufferMemory);

        
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, engineModLoader.sceneMeshes[0].meshBuffer.indexBuffer, 
        engineModLoader.sceneMeshes[0].meshBuffer.indexBufferMemory);

        CopyBuffer(stagingBuffer, engineModLoader.sceneMeshes[0].meshBuffer.indexBuffer, bufferSize);

        vkDestroyBuffer(engineDevice.logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, stagingBufferMemory, nullptr);
    }

    void Renderer::CreateDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding animLayoutBinding{};
        animLayoutBinding.binding = 2;
        animLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        animLayoutBinding.descriptorCount = 1;
        animLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        std::array<VkDescriptorSetLayoutBinding, 3> bindings = {uboLayoutBinding, samplerLayoutBinding, animLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(engineDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void Renderer::CreateDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;

        if (vkCreateDescriptorPool(engineDevice.logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void Renderer::CreateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(engineDevice.logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = transformationUBuffers.uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorBufferInfo animBufferInfo{};
            animBufferInfo.buffer = animUBuffers.uniformBuffers[i];
            animBufferInfo.offset = 0;
            animBufferInfo.range = sizeof(BoneMatricesUniformType);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture.textureImageView;
            imageInfo.sampler = texture.textureSampler;

            std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;


            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = descriptorSets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pBufferInfo = &animBufferInfo;

            vkUpdateDescriptorSets(engineDevice.logicalDevice, 
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
        }
    }

    void Renderer::UpdateUniformBuffer(uint32_t currentImage)
    {
        engineTransform.Move(glm::vec3(-4.0f, 0.0f, -0.8f));
        engineTransform.Scale(glm::vec3(0.03f), engineTransform.ubo.model);
        
        camera.UpdateViewMatrix(engineTransform.ubo.view);

        engineTransform.ubo.proj = glm::perspective(glm::radians(45.0f), engineDevice.swapChainExtent.width / 
        (float) engineDevice.swapChainExtent.height, 0.07f, 1000.0f);

        engineTransform.ubo.proj[1][1] *= -1;

        memcpy(animUBuffers.uniformBuffersMapped[currentImage], 
        &UNBoneMatrices, sizeof(UNBoneMatrices));
        
        memcpy(transformationUBuffers.uniformBuffersMapped[currentImage], 
        &engineTransform.ubo, sizeof(engineTransform.ubo));

        
    }

    VkCommandBuffer Renderer::BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        if(vkAllocateCommandBuffers(engineDevice.logicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if(vkBeginCommandBuffer(commandBuffer, &beginInfo)!= VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin command buffer!");
        }

        return commandBuffer;
    }

    void Renderer::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        if(vkEndCommandBuffer(commandBuffer)!= VK_SUCCESS)
        {
            throw std::runtime_error("failed to end command buffer!");
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        if(vkQueueSubmit(engineDevice.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)!= VK_SUCCESS)
        {
             throw std::runtime_error("failed to submit queue!");
        }
        if(vkQueueWaitIdle(engineDevice.graphicsQueue)!= VK_SUCCESS)
        {
             throw std::runtime_error("failed to queue wait!");
        }

        vkFreeCommandBuffers(engineDevice.logicalDevice, commandPool, 1, &commandBuffer);
    }

    void Renderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (HasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        EndSingleTimeCommands(commandBuffer);
    }

    void Renderer::CreateDepthResources()
    {
        VkFormat depthFormat = FindDepthFormat();
        texture.CreateImage(engineDevice.swapChainExtent.width, engineDevice.swapChainExtent.height, 
        depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = engineDevice.CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        TransitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    VkFormat Renderer::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, 
    VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(engineDevice.physicalDevice, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
        
    }

    VkFormat Renderer::FindDepthFormat()
    {
        return FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool Renderer::HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    uint32_t Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(engineDevice.physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void Renderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
    VkBuffer &buffer, VkDeviceMemory &bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(engineDevice.logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(engineDevice.logicalDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(engineDevice.logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        if (vkBindBufferMemory(engineDevice.logicalDevice, buffer, bufferMemory, 0)!= VK_SUCCESS)
        {
            throw std::runtime_error("failed to bind buffer memory!");
        }
    }

    void Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer);
    }

    void Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };
        vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
);
        EndSingleTimeCommands(commandBuffer);
    }

    Renderer::~Renderer()
    {
        std::cout << "Destruction Renderer... \n";
        vkDestroyImageView(engineDevice.logicalDevice, depthImageView, nullptr);
        vkDestroyImage(engineDevice.logicalDevice, depthImage, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, depthImageMemory, nullptr);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(engineDevice.logicalDevice, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(engineDevice.logicalDevice, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(engineDevice.logicalDevice, inFlightFences[i], nullptr);
        }
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(engineDevice.logicalDevice, framebuffer, nullptr);
        }
        vkDestroyCommandPool(engineDevice.logicalDevice, commandPool, nullptr);
        vkDestroyRenderPass(engineDevice.logicalDevice, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(engineDevice.logicalDevice, transformationUBuffers.uniformBuffers[i], nullptr);
            vkFreeMemory(engineDevice.logicalDevice, transformationUBuffers.uniformBuffersMemory[i], nullptr);
            vkDestroyBuffer(engineDevice.logicalDevice, animUBuffers.uniformBuffers[i], nullptr);
            vkFreeMemory(engineDevice.logicalDevice, animUBuffers.uniformBuffersMemory[i], nullptr);
        }
        vkDestroyDescriptorPool(engineDevice.logicalDevice, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(engineDevice.logicalDevice, descriptorSetLayout, nullptr);
    }
    Renderer::Renderer(Renderer &&other) noexcept
    {
        currentFrame = std::move(other.currentFrame);
        renderPass = std::move(other.renderPass);
        swapChainFramebuffers = std::move(other.swapChainFramebuffers);
        commandPool = std::move(other.commandPool);
        commandBuffers = std::move(other.commandBuffers);
        imageAvailableSemaphores = std::move(other.imageAvailableSemaphores);
        renderFinishedSemaphores = std::move(other.renderFinishedSemaphores);
        inFlightFences = std::move(other.inFlightFences);
        descriptorSetLayout = std::move(other.descriptorSetLayout);
        framebufferResized = std::move(other.framebufferResized);
        descriptorPool = std::move(other.descriptorPool);
        transformationUBuffers.uniformBuffers = std::move(other.transformationUBuffers.uniformBuffers);
        transformationUBuffers.uniformBuffersMemory = std::move(other.transformationUBuffers.uniformBuffersMemory);
        transformationUBuffers.uniformBuffersMapped = std::move(other.transformationUBuffers.uniformBuffersMapped);
        depthImage = std::move(other.depthImage);
        depthImageMemory = std::move(other.depthImageMemory);
        depthImageView = std::move(other.depthImageView);
        descriptorSets = std::move(other.descriptorSets);

        //CLEAN
        vkDestroyRenderPass(engineDevice.logicalDevice, other.renderPass, nullptr);
        for (auto framebuffer : other.swapChainFramebuffers) 
        {
            vkDestroyFramebuffer(engineDevice.logicalDevice, framebuffer, nullptr);
        }
        vkDestroyCommandPool(engineDevice.logicalDevice, other.commandPool, nullptr);
        for (size_t i = 0; i < other.MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroySemaphore(engineDevice.logicalDevice, other.renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(engineDevice.logicalDevice, other.imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(engineDevice.logicalDevice, other.inFlightFences[i], nullptr);
        }
        vkDestroyDescriptorSetLayout(engineDevice.logicalDevice, other.descriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(engineDevice.logicalDevice, other.descriptorPool, nullptr);

        for (size_t i = 0; i < other.MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroyBuffer(engineDevice.logicalDevice, other.transformationUBuffers.uniformBuffers[i], nullptr);
            vkFreeMemory(engineDevice.logicalDevice, other.transformationUBuffers.uniformBuffersMemory[i], nullptr);
        }

        vkDestroyImageView(engineDevice.logicalDevice, other.depthImageView, nullptr);
        vkDestroyImage(engineDevice.logicalDevice, other.depthImage, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, other.depthImageMemory, nullptr);

    }
    Renderer &Renderer::operator=(Renderer &&other) noexcept
    {
        currentFrame = std::move(other.currentFrame);
        renderPass = std::move(other.renderPass);
        swapChainFramebuffers = std::move(other.swapChainFramebuffers);
        commandPool = std::move(other.commandPool);
        commandBuffers = std::move(other.commandBuffers);
        imageAvailableSemaphores = std::move(other.imageAvailableSemaphores);
        renderFinishedSemaphores = std::move(other.renderFinishedSemaphores);
        inFlightFences = std::move(other.inFlightFences);
        descriptorSetLayout = std::move(other.descriptorSetLayout);
        framebufferResized = std::move(other.framebufferResized);
        descriptorPool = std::move(other.descriptorPool);
        transformationUBuffers.uniformBuffers = std::move(other.transformationUBuffers.uniformBuffers);
        transformationUBuffers.uniformBuffersMemory = std::move(other.transformationUBuffers.uniformBuffersMemory);
        transformationUBuffers.uniformBuffersMapped = std::move(other.transformationUBuffers.uniformBuffersMapped);
        depthImage = std::move(other.depthImage);
        depthImageMemory = std::move(other.depthImageMemory);
        depthImageView = std::move(other.depthImageView);
        descriptorSets = std::move(other.descriptorSets);

        //CLEAN
        vkDestroyRenderPass(engineDevice.logicalDevice, other.renderPass, nullptr);
        for (auto framebuffer : other.swapChainFramebuffers) 
        {
            vkDestroyFramebuffer(engineDevice.logicalDevice, framebuffer, nullptr);
        }
        vkDestroyCommandPool(engineDevice.logicalDevice, other.commandPool, nullptr);
        for (size_t i = 0; i < other.MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroySemaphore(engineDevice.logicalDevice, other.renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(engineDevice.logicalDevice, other.imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(engineDevice.logicalDevice, other.inFlightFences[i], nullptr);
        }
        vkDestroyDescriptorSetLayout(engineDevice.logicalDevice, other.descriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(engineDevice.logicalDevice, other.descriptorPool, nullptr);

        for (size_t i = 0; i < other.MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroyBuffer(engineDevice.logicalDevice, other.transformationUBuffers.uniformBuffers[i], nullptr);
            vkFreeMemory(engineDevice.logicalDevice, other.transformationUBuffers.uniformBuffersMemory[i], nullptr);
        }

        vkDestroyImageView(engineDevice.logicalDevice, other.depthImageView, nullptr);
        vkDestroyImage(engineDevice.logicalDevice, other.depthImage, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, other.depthImageMemory, nullptr);

        return *this;
    }
}