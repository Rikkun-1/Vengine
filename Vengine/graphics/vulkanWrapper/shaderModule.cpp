#include "ShaderModule.h"

#include <stdexcept>

///////////////////////// SHADER MODULE BEG //////////////////////////////

ShaderModule::ShaderModule()
{
    handle = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
    entry  = "main";
    stage  = VK_SHADER_STAGE_ALL;
}

void ShaderModule::setDevice(const LogicalDevice &device)
{
    this->device = &device;
}

void ShaderModule::create(const std::vector<char>     &code,
                                VkShaderStageFlagBits  stage,
                          const std::string           &entry)
{
    this->stage = stage;
    this->entry = entry;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());
    
    if(vkCreateShaderModule(device->handle, &createInfo, nullptr, &handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");
}

void ShaderModule::destroy()
{
    if(this->handle)
    {
        vkDestroyShaderModule(device->handle, this->handle, nullptr);
        handle = VK_NULL_HANDLE;
        entry = "main";
        stage = VK_SHADER_STAGE_ALL;
    }
}

ShaderModule::~ShaderModule()
{
    destroy();
}

///////////////////////// SHADER MODULE END //////////////////////////////