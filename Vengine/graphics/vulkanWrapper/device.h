#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>


struct LogicalDevice
{
    VkDevice         handle;
    VkPhysicalDevice physicalDevice;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
};


VkPhysicalDevice pickPhysicalDevice(VkInstance                      instance,
                                    VkSurfaceKHR                    surface,
                                    const std::vector<const char *> &requiredExtensions);


LogicalDevice createLogicalDevice(VkInstance                      instance,
                                  VkPhysicalDevice                physicalDevice,
                                  VkSurfaceKHR                    surface,
                                  const std::vector<const char *> &requiredExtensions);