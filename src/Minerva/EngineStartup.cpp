#include "EngineStartup.h"
#include "EngineVars.h"

namespace Minerva
{
    VulkanInstance engineInstance;
    Window windowInstance;
    DebugManager debugLayer;
    Device engineDevice;

    void EngineStartup::RunEngine()
    {
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
    }
    void EngineStartup::Loop()
    {
        while (!glfwWindowShouldClose(windowInstance.window)) {
            glfwPollEvents();
        }
    }
}
