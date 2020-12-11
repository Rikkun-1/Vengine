#pragma once

#include <vulkan/vulkan.h>
#include "device.h"

struct CommandPool
{
    VkCommandPool        handle;
    
    LogicalDevice       *device;

    CommandPool(LogicalDevice  *device);

    void create();

    void setDevice(LogicalDevice  *device);

    void allocateCommandBuffers(int              amount,
                                VkCommandBuffer *commandBuffers);

    void freeCommandBuffers(int               amount,
                            VkCommandBuffer  *commandBuffers);

    void destroy();

private:
    uint32_t  queueFamilyIndex;
};