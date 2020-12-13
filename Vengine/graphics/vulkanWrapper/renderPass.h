#pragma once

#include "vulkan/vulkan.h"

#include "device.h"

VkRenderPass createRenderPass(const LogicalDevice &device,
                                    VkFormat       swapChainImageFormat);