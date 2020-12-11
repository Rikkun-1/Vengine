#pragma once

#include <array>

#include "device.h"

VkFramebuffer createFrameBuffer(const LogicalDevice              &device,
                                VkRenderPass                      renderPass,
                                const VkExtent3D                 &extent,
                                const std::array<VkImageView, 2> &attachments);