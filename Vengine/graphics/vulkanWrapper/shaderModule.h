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
    
    void setDevice(const LogicalDevice    &device);

    void create(const std::vector<char>     &code,
                      VkShaderStageFlagBits  stage,
                const std::string           &entry);

    void destroy();

    ~ShaderModule();
};