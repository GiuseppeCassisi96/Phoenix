#pragma once
#include "vulkan/vulkan.h"
#include "imgui.h"
#include "imconfig.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "string"
namespace Minerva
{
    class MinervaUI
    {
    public:
        const std::string FONTS_PATH = "C:/UNIMI/TESI/Phoenix/src/Minerva/Fonts/";
        ImFont* font;

        MinervaUI() = default;
        ~MinervaUI();

        MinervaUI(const MinervaUI& other) = delete;
        MinervaUI& operator=(const MinervaUI other) = delete;

        MinervaUI(MinervaUI&& other) noexcept;
        MinervaUI& operator=(MinervaUI&& other) noexcept;
        void SetupUI();
        void RenderUI(VkCommandBuffer& currentCmdBuffer);
    };
    
    
}