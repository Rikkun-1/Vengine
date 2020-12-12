#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include <iostream>

#include "vertex.h"
#include "shaderModule.h"
#include "device.h"

VkPipeline createGraphicsPipeline(const LogicalDevice         &device,
                                  VkExtent2D                  &swapChainExtent,
                                  VkRenderPass                renderPass,
                                  const ShaderModule          &vertexShader,
                                  const ShaderModule          &fragmentShader,
                                  const VkDescriptorSetLayout &descriptorSetLayout,
                                  const VkPipelineLayout      &pipelineLayout);