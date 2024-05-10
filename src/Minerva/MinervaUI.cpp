#include "EngineVars.h"
#include "MinervaUI.h"
#include "EngineStartup.h"



namespace Minerva
{
    MinervaUI::~MinervaUI()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    MinervaUI::MinervaUI(MinervaUI &&other) noexcept
    {
        font = std::move(other.font);

        delete other.font;
    }

    MinervaUI &MinervaUI::operator=(MinervaUI &&other) noexcept
    {
        font = std::move(other.font);

        delete other.font;
        return *this;
    }

    void MinervaUI::SetupUI(EngineStartup& engine)
    {
        this->engine = &engine;
        QueueFamilyIndices indices = engineDevice.FindQueueFamilies(engineDevice.physicalDevice,
        windowInstance.windowSurface);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;   

        font = io.Fonts->AddFontFromFileTTF((FONTS_PATH + 
        "Roboto-Medium.ttf").c_str(),20.0f);

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4 windowBGcolor = {0.5f, 0.0f, 0.06f, 1.0f};
        ImVec4 borderColor = {0.0f, 0.0f, 0.0f, 0.5f};
        style.Colors[ImGuiCol_WindowBg] = windowBGcolor;
        style.Colors[ImGuiCol_Border] = borderColor;
        style.WindowRounding = 15.0f;


        ImGui_ImplGlfw_InitForVulkan(windowInstance.window, true);

        ImGui_ImplVulkan_InitInfo imGuiImplInfo {};
        imGuiImplInfo.Instance = engineInstance.instance;
        imGuiImplInfo.QueueFamily = indices.graphicsFamily.value();
        imGuiImplInfo.Queue = engineDevice.graphicsQueue;
        imGuiImplInfo.DescriptorPool = engineRenderer.descriptorPool;
        imGuiImplInfo.RenderPass = engineRenderer.renderPass;
        imGuiImplInfo.Device = engineDevice.logicalDevice;
        imGuiImplInfo.MSAASamples = engineDevice.msaaSamples;
        imGuiImplInfo.PhysicalDevice = engineDevice.physicalDevice;
        imGuiImplInfo.ImageCount = engineRenderer.MAX_FRAMES_IN_FLIGHT;
        imGuiImplInfo.MinImageCount = engineRenderer.MAX_FRAMES_IN_FLIGHT;
        imGuiImplInfo.PipelineCache = nullptr; //!TO-DO Implement a pipeline cache to increase performance
        
        ImGui_ImplVulkan_Init(&imGuiImplInfo);
        
    }

    void MinervaUI::RenderUI(VkCommandBuffer& currentCmdBuffer)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::PushFont(font);
            ImGui::Begin("Minerva Engine");                          
            ImGui::Text("Minerva Engine Rendering Info: ");
            ImGui::NewLine();

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / 
            ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("Number of bones: %d", engineModLoader.info.numberOfBones);
            ImGui::Text("Number of triangles: %d", engineModLoader.info.numberOfPolygons  * engineModLoader.instanceNumber);
            ImGui::Text("Number of vertices: %d", engineModLoader.info.numberOfVertices  * engineModLoader.instanceNumber);
            ImGui::Text("Number of instances: %d", engineModLoader.instanceNumber);
            if(engineModLoader.sceneMeshes[0].typeOfMesh == Mesh::MeshType::Skeletal)
            {
                if(ImGui::Button("Idle"))
                {
                    this->engine->animator.PlayAnimation(&this->engine->animations[0]);
                }

                if(ImGui::Button("Walk"))
                {
                    this->engine->animator.PlayAnimation(&this->engine->animations[1]);
                }

                if(ImGui::Button("Run"))
                {
                    this->engine->animator.PlayAnimation(&this->engine->animations[2]);
                }
            }    
            ImGui::PopFont();
            ImGui::End();
        }
        
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), currentCmdBuffer);
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    
}
