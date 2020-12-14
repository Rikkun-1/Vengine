#pragma once

#include <vulkan/vulkan.h>

#include "device.h"

struct CommandPool
{   
    VkCommandPool         handle;
    
    const LogicalDevice  *device;

    CommandPool();
    CommandPool(const LogicalDevice *device);

    void create();

    void setDevice(const LogicalDevice *device);

    void allocateCommandBuffers(uint32_t         amount,
                                VkCommandBuffer *commandBuffers);

    void freeCommandBuffers(uint32_t          amount,
                            VkCommandBuffer  *commandBuffers);

    void destroy();

private:
    uint32_t  queueFamilyIndex;
};