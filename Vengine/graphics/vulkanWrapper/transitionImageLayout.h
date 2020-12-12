#pragma once

#include  <vulkan/Vulkan.h>

#include "commandPool.h"
#include "commandBuffer.h"
#include "image.h"

void transitionImageLayout(CommandPool    &commandPool,
                           Image          &image,
                           VkFormat       format,
                           VkImageLayout  oldLayout,
                           VkImageLayout  newLayout);
                           