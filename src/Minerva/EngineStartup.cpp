#include "EngineStartup.h"

namespace Minerva
{
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
        engineDevice.PickMostSuitableDevice(engineInstance.instance);
        engineDevice.PrintInfoDeviceSelected();
        engineDevice.CreateLogicalDevice(debugLayer);
    }
    void EngineStartup::Loop()
    {
        while (!glfwWindowShouldClose(windowInstance.window)) {
        glfwPollEvents();
    }
    }
}
