#include "ShaderModule.h"

#include <stdexcept>;

ShaderModule::ShaderModule(VkDevice                    logicalDevice,
                           const std::vector<char>     &code,
                           enum VkShaderStageFlagBits  stage,
                           std::string                 entry)
{
    this->logicalDevice = logicalDevice;
    this->stage         = stage;
    this->entry         = entry;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");
}

ShaderModule::~ShaderModule()
{
    vkDestroyShaderModule(logicalDevice, this->handle, nullptr);
}