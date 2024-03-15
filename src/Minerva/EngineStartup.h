#pragma once
#include "VulkanInstance.h"
#include "DebugManager.h"
#include "Window.h"
#include "Device.h"
namespace Minerva
{
    class EngineStartup
    {
    public:
        void RunEngine();
    private:
        VulkanInstance engineInstance;
        Window windowInstance;
        DebugManager debugLayer;
        Device engineDevice;
        void Start();
        void Loop();
    };
    
    
}