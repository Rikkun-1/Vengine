#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>

void setupPipelineLayoutInfo(const VkDescriptorSetLayout       &descriptorSetLayout,
                                   VkPipelineLayoutCreateInfo  &pipelineLayoutInfo);

VkPipelineLayout createPipelineLayout(VkDevice                    logicalDevice,
                                      const VkDescriptorSetLayout &descriptorSetLayout);