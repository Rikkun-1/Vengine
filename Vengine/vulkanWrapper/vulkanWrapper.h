#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>

#include "setupValidationLayers.h"
#include "extensionsCheck.h"
#include "setupDevices.h"

#include "setupSwapchain.h"
#include "buffer.h"
#include "commandBuffer.h"

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

void createInstance(const std::vector<const char *> &validationLayers,
                    const std::vector<const char *> &instanceExtensions,
                    VkInstance                      &instance);


void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT &debugMessenger);


VkPhysicalDevice pickPhysicalDevice(VkInstance                      instance,
                                    VkSurfaceKHR                    surface,
                                    const std::vector<const char *> &requiredExtenisons);


void createLogicalDevice(VkPhysicalDevice          physicalDevice,
                         VkDevice                  &logicalDevice,
                         VkSurfaceKHR              surface,
                         std::vector<const char *> &requiredExtenisons,
                         VkQueue                   &graphicsQueue,
                         VkQueue                   &presentQueue);


void createSwapChain(GLFWwindow           *pWindow,
                     VkPhysicalDevice     physicalDevice,
                     VkDevice             logicalDevice,
                     VkSurfaceKHR         surface,
                     VkSwapchainKHR       &swapChain,
                     std::vector<VkImage> &swapChainImages,
                     VkFormat             &swapChainImageFormat,
                     VkExtent2D           &swapChainExtent);

void createImageViews(VkDevice                   logicalDevice,
                      VkFormat                   swapChainImageFormat,
                      const std::vector<VkImage> &swapChainImages,
                      std::vector<VkImageView>   &swapChainImageViews);

///////////////////////////////////////////////////////////

void createRenderPass(VkDevice     logicalDevice,
                      VkFormat     swapChainImageFormat,
                      VkRenderPass &renderPass);

///////////////////////////////////////////////////////////

VkShaderModule createShaderModule(VkDevice                logicalDevice,
                                  const std::vector<char> &code);


void createGraphicsPipeline(VkDevice              logicalDevice,
                            VkExtent2D            swapChainExtent,
                            VkRenderPass          renderPass,
                            VkPipelineLayout      &pipelineLayout,
                            VkPipeline            &graphicsPipeline,
                            VkDescriptorSetLayout &descriptorSetLayout);


static std::vector<char> readFile(const std::string &filename);

///////////////////////////////////////////////////////////

void createFramebuffers(VkDevice                       logicalDevice,
                        VkRenderPass                   renderPass,
                        VkExtent2D                     swapChainExtent,
                        const std::vector<VkImageView> &swapChainImageViews,
                        std::vector<VkFramebuffer>     &swapChainFramebuffers);

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
                          std::vector<VkBuffer>        &uniformBuffers);
