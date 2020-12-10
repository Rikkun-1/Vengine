#pragma once

#include <vulkan/vulkan.h>

#include <vector>

void createDescriptorPool(VkDevice                   logicalDevice,
                          const std::vector<VkImage> swapChainImages,
                          VkDescriptorPool           &descriptorPool);

void createDescriptorSets(VkDevice                     logicalDevice,
                          const std::vector<VkImage>   swapChainImages,
                          VkDescriptorPool             descriptorPool,
                          VkDescriptorSetLayout        descriptorSetLayout,
                          std::vector<VkDescriptorSet> &descriptorSets,
                          std::vector<VkBuffer>        &uniformBuffers,
                          VkImageView                  textureImageView,
                          VkSampler                    textureSampler);