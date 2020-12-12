#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool has_value() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct LogicalDevice
{
    VkDevice           handle;
    VkPhysicalDevice   physicalDevice;
    
    QueueFamilyIndices familyIndices;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
};


SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice,
                                              VkSurfaceKHR     surface);


QueueFamilyIndices findQueueFamilies(VkPhysicalDevice  physicalDevice,
                                      VkSurfaceKHR     surface);


VkPhysicalDevice pickPhysicalDevice(VkInstance                      instance,
                                    VkSurfaceKHR                    surface,
                                    const std::vector<const char *> &requiredExtensions);


LogicalDevice createLogicalDevice(VkInstance                      instance,
                                  VkPhysicalDevice                physicalDevice,
                                  VkSurfaceKHR                    surface,
                                  const std::vector<const char *> &requiredExtensions);