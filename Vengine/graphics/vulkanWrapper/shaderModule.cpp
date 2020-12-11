#include "ShaderModule.h"

#include <stdexcept>

ShaderModule::ShaderModule()
{
    handle = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
    entry  = "main";
    stage  = VK_SHADER_STAGE_ALL;
}

ShaderModule::ShaderModule(const LogicalDevice               &device,
                           const std::vector<char>     &code,
                           enum VkShaderStageFlagBits   stage,
                           const std::string           &entry)
{
    this->device = &device;
    this->stage  = stage;
    this->entry  = entry;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(device.handle, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");
}

ShaderModule::~ShaderModule()
{
    vkDestroyShaderModule(device->handle, this->handle, nullptr);
}