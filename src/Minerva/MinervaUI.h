#pragma once
#include "imgui.h"
#include "imconfig.h"
#include "vulkan/vulkan.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "string"
namespace Minerva
{
    class EngineStartup;
    class MinervaUI
    {
    public:
        const std::string FONTS_PATH = "C:/UNIMI/TESI/Phoenix/src/Minerva/Fonts/";
        ImFont* font;
        EngineStartup* engine;
        float frame = 0.0f;
        float sumFramerates = 0.0f; 

        MinervaUI() = default;
        ~MinervaUI();

        MinervaUI(const MinervaUI& other) = delete;
        MinervaUI& operator=(const MinervaUI other) = delete;

        MinervaUI(MinervaUI&& other) noexcept;
        MinervaUI& operator=(MinervaUI&& other) noexcept;
        void SetupUI(EngineStartup& engine);
        void RenderUI(VkCommandBuffer& currentCmdBuffer);
    };
    
    
}