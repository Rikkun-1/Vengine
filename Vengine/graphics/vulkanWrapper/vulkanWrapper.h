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

///////////////////////////////////////////////////////////

void createSyncObjects(VkDevice                   logicalDevice,
                       int                        MAX_FRAMES_IN_FLIGHT,
                       const std::vector<VkImage> &swapChainImages,
                       std::vector<VkSemaphore>   &imageAvailableSemaphores,
                       std::vector<VkSemaphore>   &renderFinishedSemaphores,
                       std::vector<VkFence>       &inFlightFences,
                       std::vector<VkFence>       &imagesInFlight);

