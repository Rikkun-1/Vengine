#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <string>
/*
* Структура, описывающая настройки для Renderer
*/
struct vkSettings
{
    int32_t windowWidth;
    int32_t windowHeight;

    char windowTitle[100] = "Vulkan";

    std::vector<const char *> validationLayers;
    std::vector<const char *> instanceExtensions;
    std::vector<const char *> deviceExtensions;

    std::string model_path;
    std::string texture_path;

    vkSettings();
};
