#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "device.h"
#include "swapChain.h"

void createCommandBuffers(VkDevice                     logicalDevice,
                          SwapChain                    &swapChain,
                          VkRenderPass                 renderPass,
                          VkPipeline                   graphicsPipeline,
                          VkPipelineLayout             pipelineLayout,
                          VkBuffer                     vertexBuffer,
                          VkBuffer                     indexBuffer,
                          std::vector<VkDescriptorSet> &descriptorSets,
                          int                          indexBufferSize,
                          VkCommandPool                commandPool,
                          std::vector<VkCommandBuffer> &commandBuffers);


VkCommandBuffer beginSingleTimeCommands(VkDevice      logicalDevice,
                                        VkCommandPool commandPool);


void endSingleTimeCommands(const LogicalDevice  &device,
                           VkCommandBuffer      commandBuffer,
                           VkCommandPool        commandPool);


VkCommandPool createCommandPool(const LogicalDevice  &device,
                                VkSurfaceKHR         surface);