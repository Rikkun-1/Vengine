#pragma once

#include "vulkan/vulkan.h"

#include "LogicalDevice.h"

void fillColorAttachmentDescription(VkAttachmentDescription &colorAttachment);

void fillDepthAttachmentDescription(VkAttachmentDescription &depthAttachment);

