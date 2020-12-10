#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>

#include "setupValidationLayers.h"
#include "extensionsCheck.h"

#include "setupPhysicalDevice.h"
#include "LogicalDevice.h"

#include "SwapChain.h"
#include "setupRenderPass.h"

#include "buffer.h"
#include "commandBuffer.h"
#include "image.h"
#include "shaderModule.h"
#include "descriptorSetLayout.h"
#include "pipelineLayout.h"






void createSwapChain(const LogicalDevice  &device,
                     VkSurfaceKHR          surface,
                     VkExtent2D           &requiredExtent,
                     SwapChain            &swapChain);


VkRenderPass createRenderPass(const LogicalDevice &device,
                              VkFormat            swapChainImageFormat);

///////////////////////////////////////////////////////////

void createGraphicsPipeline(VkDevice              logicalDevice,
                            VkExtent2D            swapChainExtent,
                            VkRenderPass          renderPass,
                            VkPipelineLayout      &pipelineLayout,
                            VkPipeline            &graphicsPipeline,
                            VkDescriptorSetLayout &descriptorSetLayout);

///////////////////////////////////////////////////////////

void createFramebuffers(VkDevice                       logicalDevice,
                        VkRenderPass                   renderPass,
                        VkExtent2D                     swapChainExtent,
                        const std::vector<VkImageView> &swapChainImageViews,
                        std::vector<VkFramebuffer>     &swapChainFramebuffers,
                        VkImageView                    depthImageView);
///////////////////////////////////////////////////////////

void createCommandPool(VkPhysicalDevice physicalDevice,
                       VkDevice         logicalDevice,
                       VkSurfaceKHR     surface, 
                       VkCommandPool    &commandPool);

///////////////////////////////////////////////////////////

void createSyncObjects(VkDevice                   logicalDevice,
                       int                        MAX_FRAMES_IN_FLIGHT,
                       const std::vector<VkImage> &swapChainImages,
                       std::vector<VkSemaphore>   &imageAvailableSemaphores,
                       std::vector<VkSemaphore>   &renderFinishedSemaphores,
                       std::vector<VkFence>       &inFlightFences,
                       std::vector<VkFence>       &imagesInFlight);

void createDescriptorSetLayout(VkDevice              logicalDevice, 
                               VkDescriptorSetLayout &descriptorSetLayout);

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
