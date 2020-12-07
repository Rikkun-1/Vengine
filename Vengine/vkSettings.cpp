#include "vkSettings.h"

vkSettings::vkSettings()
{
    windowWidth  = 800;
    windowHeight = 600;

    validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    instanceExtensions = {};
    deviceExtensions = { 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME 
    };
};