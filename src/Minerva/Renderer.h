#pragma once
#include "vulkan/vulkan.h"
namespace Minerva
{
    class Renderer
    {
    public:
        VkRenderPass renderPass;
        void CreateRenderPass();
        Renderer() = default;
        ~Renderer();
    };
    
    
}