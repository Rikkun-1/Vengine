#include "descriptorSetLayout.h"

void setupDescriptorSetLayoutBinding(uint32_t                     binding,
                                     uint32_t                     count,
                                     VkDescriptorType             descriptorType,
                                     VkShaderStageFlags           shaderStage,
                                     VkDescriptorSetLayoutBinding &uboLayoutBinding)
{
    uboLayoutBinding.binding         = binding;
    uboLayoutBinding.descriptorCount = count;
    uboLayoutBinding.descriptorType  = descriptorType;
    // каким стадиям будет доступно данное прикрепление?
    // можно добавить несколько стадий используя |
    // или VK_SHADER_STAGE_ALL_GRAPHICS если данное прикрепление требуется
    // на всех стадиях
    uboLayoutBinding.stageFlags      = shaderStage;

    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
}

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice  logicalDevice)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    setupDescriptorSetLayoutBinding(0, //binding
                                    1, //count
                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                    VK_SHADER_STAGE_VERTEX_BIT,
                                    uboLayoutBinding);

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    setupDescriptorSetLayoutBinding(1, 
                                    1, 
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                    VK_SHADER_STAGE_FRAGMENT_BIT,
                                    uboLayoutBinding);

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};


    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

    VkDescriptorSetLayout descriptorSetLayout;
    if(vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");

    return descriptorSetLayout;
}
