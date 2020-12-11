#pragma once

#include "device.h"
#include "swapChain.h"

VkFramebuffer createFrameBuffer(const LogicalDevice              &device,
                                VkRenderPass                      renderPass,
                                const VkExtent3D                 &extent,
                                const std::array<VkImageView, 2> &attachments);