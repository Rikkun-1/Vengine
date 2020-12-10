#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <stdexcept>

void setupDescriptorSetLayoutBinding(int                          binding,
                                     int                          count,
                                     VkDescriptorType             descriptorType,
                                     VkShaderStageFlags           shaderStage,
                                     VkDescriptorSetLayoutBinding &uboLayoutBinding);

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice          logicalDevice);