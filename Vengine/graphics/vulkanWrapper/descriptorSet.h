#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <array>
#include <vector>

#include "device.h"
#include "buffer.h"

void createDescriptorSets(const LogicalDevice          &device,
                          VkDescriptorPool             descriptorPool,
                          VkDescriptorSetLayout        descriptorSetLayout,
                          std::vector<VkDescriptorSet> &descriptorSets,
                          std::vector<Buffer>          &uniformBuffers,
                          VkImageView                  textureImageView,
                          VkSampler                    textureSampler,
                          int                          amount);