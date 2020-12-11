#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "device.h"

struct ShaderModule
{
    VkShaderModule              handle;
    const LogicalDevice        *device;
    std::string                 entry;
    enum VkShaderStageFlagBits  stage;

    ShaderModule();
    ShaderModule(const LogicalDevice        &device,
                 const std::vector<char>    &code,
                 enum VkShaderStageFlagBits stage,
                 const std::string          &entry);

    ~ShaderModule();
};