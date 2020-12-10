#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

struct ShaderModule
{
    VkShaderModule             handle = VK_NULL_HANDLE;
    std::string                entry  = "main";
    VkDevice                   logicalDevice;
    enum VkShaderStageFlagBits stage;

    ShaderModule(VkDevice                   logicalDevice,
                 const std::vector<char>    &code,
                 enum VkShaderStageFlagBits stage,
                 std::string                entry);

    ~ShaderModule();
};