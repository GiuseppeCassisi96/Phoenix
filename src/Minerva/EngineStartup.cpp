#include "EngineStartup.h"
#include "EngineVars.h"
#include <iostream>

namespace Minerva
{
    VulkanInstance engineInstance;
    Window windowInstance;
    DebugManager debugLayer;
    Device engineDevice;
    EnginePipeline enginePipeline;
    Renderer engineRenderer;
    Mesh engineMesh;

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
        enginePipeline.CreatePipeline("vert", "frag");
        engineRenderer.CreateFramebuffers();
        engineRenderer.CreateCommandPool();
        engineRenderer.CreateVertexBuffer();
        engineRenderer.CreateIndexBuffer();
        engineRenderer.CreateCommandBuffer();
        engineRenderer.CreateSyncObjects();
    }
    void EngineStartup::Loop()
    {
        while (!glfwWindowShouldClose(windowInstance.window)) {
            glfwPollEvents();
            engineRenderer.DrawFrame();
        }
        vkDeviceWaitIdle(engineDevice.logicalDevice);
    }
}
