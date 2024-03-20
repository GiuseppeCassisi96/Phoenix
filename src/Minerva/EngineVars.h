#pragma once
#include "VulkanInstance.h"
#include "DebugManager.h"
#include "Window.h"
#include "Device.h"
#include "EnginePipeline.h"
#include "Renderer.h"
#include "Mesh.h"
namespace Minerva
{
    extern VulkanInstance engineInstance;
    extern Window windowInstance;
    extern DebugManager debugLayer;
    extern Device engineDevice;
    extern EnginePipeline enginePipeline;
    extern Renderer engineRenderer;
    extern Mesh engineMesh;
}
