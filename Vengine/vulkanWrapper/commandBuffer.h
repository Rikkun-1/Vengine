#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>

void createCommandBuffers(const VkDevice				   &logicalDevice,
						  const VkExtent2D				   &swapChainExtent,
						  const std::vector<VkFramebuffer> &swapChainFramebuffers,
						  const VkPipeline				   &graphicsPipeline,
						  const VkRenderPass			   &renderPass,
						  VkBuffer						   &vertexBuffer,
						  VkBuffer						   &indexBuffer,
						  VkCommandPool					   &commandPool,
						  std::vector<VkCommandBuffer>	   &commandBuffers,
						  std::vector<VkDescriptorSet>	   &descriptorSets,
						  VkPipelineLayout				   &pipelineLayout,
						  int								indexBufferSize);


VkCommandBuffer beginSingleTimeCommands(const VkDevice &logicalDevice,
										VkCommandPool  &commandPool);


void endSingleTimeCommands(VkCommandBuffer	commandBuffer,
						   const VkDevice	&logicalDevice,
						   VkCommandPool	&commandPool,
						   VkQueue			graphicsQueue);