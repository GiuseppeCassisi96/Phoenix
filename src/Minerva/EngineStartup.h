#pragma once
#include "VulkanInstance.h"
#include "DebugManager.h"
#include "Window.h"
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
        void Start();
        void Loop();
    };
    
    
}