#pragma once
#include "VulkanInstance.h"
#include "DebugManager.h"
#include "Window.h"
#include "Device.h"
namespace Minerva
{
    extern VulkanInstance engineInstance;
    extern Window windowInstance;
    extern DebugManager debugLayer;
    extern Device engineDevice;
}
