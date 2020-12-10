#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "LogicalDevice.h"

struct SwapChain
{
    VkSwapchainKHR           handle;
    std::vector<VkImage>     images;
    std::vector<VkImageView> imageViews;
    VkFormat                 imageFormat;
    VkExtent2D               extent;
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice,
                                              VkSurfaceKHR     surface);


VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);


VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);


VkExtent2D chooseSwapExtent(VkExtent2D                     requiredExtent,
                            const VkSurfaceCapabilitiesKHR &capabilities);


void createImageViews(LogicalDevice device,
                      SwapChain     swapChain);