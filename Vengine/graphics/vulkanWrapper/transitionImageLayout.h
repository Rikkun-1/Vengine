#pragma once

#include  <vulkan/Vulkan.h>

#include "commandPool.h"

void transitionImageLayout(CommandPool    &commandPool,
                           Image          image,
                           VkFormat       format,
                           VkImageLayout  oldLayout,
                           VkImageLayout  newLayout);