#pragma once

#include  <vulkan/Vulkan.h>

#include "commandPool.h"
#include "commandBuffer.h"
#include "image.h"

struct Image;
struct CommandPool;

void transitionImageLayout(CommandPool    &commandPool,
                           Image          &image,
                           VkFormat       format,
                           VkImageLayout  oldLayout,
                           VkImageLayout  newLayout);
                           