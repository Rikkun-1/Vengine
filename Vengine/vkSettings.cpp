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




    model_path = "models/LowPolyCars.obj";
    texture_path = "textures/Car Texture.png";

    model_path = "models/suzanne.obj";
    texture_path = "textures/shapes.jpg";

    model_path = "models/viking_room.obj";
    texture_path = "textures/viking_room.png";
};