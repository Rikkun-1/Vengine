#include "pipelineLayout.h"

void setupPipelineLayoutInfo(const VkDescriptorSetLayout       &descriptorSetLayout,
                                   VkPipelineLayoutCreateInfo  &pipelineLayoutInfo)
{
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;

    pipelineLayoutInfo.pushConstantRangeCount = 0;       // Optional
    pipelineLayoutInfo.pPushConstantRanges    = nullptr; // Optional
}

VkPipelineLayout createPipelineLayout(VkDevice                    logicalDevice,
                                      const VkDescriptorSetLayout &descriptorSetLayout)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    setupPipelineLayoutInfo(descriptorSetLayout, pipelineLayoutInfo);

    VkPipelineLayout pipelineLayout;
    if(vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to setup pipeline layout!");

    return pipelineLayout;
}