#pragma once

#include <vulkan/vulkan.h>
#include "device.h"

struct CommandPool
{
    VkCommandPool        handle;
    
    LogicalDevice       *device;

    CommandPool(const LogicalDevice  &device);

    void create();

    void allocateCommandBuffers(uint32_t         amount,
                                VkCommandBuffer  *commandBuffers);

    void destroy();

private:
    uint32_t  queueFamilyIndex;
};