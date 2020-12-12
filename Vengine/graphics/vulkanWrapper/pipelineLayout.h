#pragma once

#include <vulkan/vulkan.h>

void setupPipelineLayoutInfo(const VkDescriptorSetLayout       &descriptorSetLayout,
                                   VkPipelineLayoutCreateInfo  &pipelineLayoutInfo);

VkPipelineLayout createPipelineLayout(VkDevice                    logicalDevice,
                                      const VkDescriptorSetLayout &descriptorSetLayout);