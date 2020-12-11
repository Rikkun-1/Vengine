#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "CommandPool.h"
#include "swapChain.h"


void writeCommandBuffersForDrawing(CommandPool                  &commandPool,
                                   SwapChain                    &swapChain,
                                   VkRenderPass                 renderPass,
                                   VkPipeline                   graphicsPipeline,
                                   VkPipelineLayout             pipelineLayout,
                                   VkBuffer                     vertexBuffer,
                                   VkBuffer                     indexBuffer,
                                   uint32_t                     indexBufferSize,
                                   std::vector<VkDescriptorSet> &descriptorSets,
                                   std::vector<VkCommandBuffer> &commandBuffers);


VkCommandBuffer beginSingleTimeCommands(CommandPool  &commandPool);


void endSingleTimeCommands(const CommandPool   &commandPool,
                           VkCommandBuffer     commandBuffer);
