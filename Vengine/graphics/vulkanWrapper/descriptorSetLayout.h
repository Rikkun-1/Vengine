#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <stdexcept>

void setupDescriptorSetLayoutBinding(uint32_t                     binding,
                                     uint32_t                     count,
                                     VkDescriptorType             descriptorType,
                                     VkShaderStageFlags           shaderStage,
                                     VkDescriptorSetLayoutBinding &uboLayoutBinding);

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice logicalDevice);