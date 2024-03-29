#include "EngineStartup.h"
#include "EngineVars.h"
#include <iostream>
#include "imgui.h"
#include "imconfig.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Minerva
{
    VulkanInstance engineInstance;
    Window windowInstance;
    DebugManager debugLayer;
    Device engineDevice;
    EnginePipeline enginePipeline;
    Renderer engineRenderer;
    Mesh engineMesh;
    Transformation engineTransform;
    TextureManager texture;
    EngineCamera camera;
    MinervaUI engineUI;

    void EngineStartup::RunEngine()
    {
        std::cout << "                                          -----------------MINERVA ENGINE-----------------\n\n";
        Start();
        Loop();
        debugLayer.DestroyDebugUtilsMessengerEXT(engineInstance.instance,debugLayer.debugMessenger,nullptr);
    }

    void EngineStartup::Start()
    {
        windowInstance.EngineInitWindow(windowInstance.WIDTH, windowInstance.HEIGHT);
        engineInstance.CreateInstance();
        debugLayer.SetupDebugMessenger(engineInstance.instance);
        windowInstance.CreateWindowSurface(engineInstance.instance);
        engineDevice.PickMostSuitableDevice(engineInstance.instance, windowInstance.windowSurface);
        engineDevice.PrintInfoDeviceSelected();
        engineDevice.CreateLogicalDevice(debugLayer, windowInstance.windowSurface);
        engineDevice.CreateSwapChain();
        engineDevice.CreateImageViews();
        engineRenderer.CreateRenderPass();
        engineRenderer.CreateDescriptorSetLayout();
        enginePipeline.CreatePipeline("vert", "frag");
        engineRenderer.CreateCommandPool();
        engineRenderer.CreateDepthResources();
        engineRenderer.CreateFramebuffers();
        texture.CreateTextureImage("SteamHammerColor.png");
        texture.CreateTextureImageView();
        texture.CreateTextureSampler();
        engineMesh.LoadModel("SteamHammer.obj");
        engineMesh.PrepareInstanceData();
        engineRenderer.CreateVertexBuffer();
        engineRenderer.CreateInstanceBuffer();
        engineRenderer.CreateIndexBuffer();
        engineRenderer.CreateUniformBuffers();
        engineRenderer.CreateDescriptorPool();
        engineRenderer.CreateDescriptorSets();
        engineRenderer.CreateCommandBuffer();
        engineRenderer.CreateSyncObjects();

        
        glfwSetInputMode(windowInstance.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        camera.SetupViewMatrix(engineTransform.ubo.view);
        glfwSetCursorPosCallback(windowInstance.window, [](GLFWwindow* window, double xpos, double ypos)
        {
            if(windowInstance.isCursorDisabled)
                camera.MouseCallback(window, xpos, ypos);
        });
        engineUI.SetupUI();
        glfwSetKeyCallback(windowInstance.window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            windowInstance.KeyPressCallback(window, key, scancode, action, mods);
        });
          
    }
    void EngineStartup::Loop()
    {
        while (!glfwWindowShouldClose(windowInstance.window)) {
            
            glfwPollEvents();
            camera.ProcessUserInput(windowInstance.window);
            engineRenderer.DrawFrame();
            
        }
        vkDeviceWaitIdle(engineDevice.logicalDevice);
    }
}
