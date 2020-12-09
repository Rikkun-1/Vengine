#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <vector>
#include <stdexcept>

void createCommandBuffers(VkDevice                     logicalDevice,
                          VkExtent2D                   swapChainExtent,
                          std::vector<VkFramebuffer>   &swapChainFramebuffers,
                          VkPipeline                   graphicsPipeline,
                          VkRenderPass                 renderPass,
                          VkBuffer                     vertexBuffer,
                          VkBuffer                     indexBuffer,
                          VkCommandPool                commandPool,
                          std::vector<VkCommandBuffer> &commandBuffers,
                          std::vector<VkDescriptorSet> &descriptorSets,
                          VkPipelineLayout             pipelineLayout,
                          int                          indexBufferSize);


VkCommandBuffer beginSingleTimeCommands(VkDevice      logicalDevice,
                                        VkCommandPool commandPool);

void endSingleTimeCommands(VkCommandBuffer commandBuffer,
                           VkDevice        logicalDevice,
                           VkCommandPool   commandPool,
                           VkQueue         graphicsQueue);
