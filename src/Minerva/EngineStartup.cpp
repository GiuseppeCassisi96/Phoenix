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
    Transformation engineTransform;
    TextureManager texture;
    EngineCamera camera;

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
        engineRenderer.CreateVertexBuffer();
        engineRenderer.CreateIndexBuffer();
        engineRenderer.CreateUniformBuffers();
        engineRenderer.CreateDescriptorPool();
        engineRenderer.CreateDescriptorSets();
        engineRenderer.CreateCommandBuffer();
        engineRenderer.CreateSyncObjects();

        camera.SetupViewMatrix(engineTransform.ubo.view);
        glfwSetCursorPosCallback(windowInstance.window, [](GLFWwindow* window, double xpos, double ypos)
        {
            camera.MouseCallback(window, xpos, ypos);
        });
        glfwSetInputMode(windowInstance.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
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
