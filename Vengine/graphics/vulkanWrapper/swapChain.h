#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "device.h"

struct SwapChain
{
    VkSwapchainKHR              handle;
    std::vector<VkImage>        images;
    std::vector<VkImageView>    imageViews;
    std::vector<VkFramebuffer>  frameBuffers;
    VkFormat                    imageFormat;
    VkExtent2D                  extent;
};


struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};


SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice,
                                              VkSurfaceKHR     surface);


void createFramebuffers(VkDevice          logicalDevice,
                        VkRenderPass      renderPass,
                        SwapChain         swapChain,
                        VkImageView       depthImageView);


void createImageViews(const LogicalDevice &device,
                      SwapChain           swapChain);


void createSwapChain(const LogicalDevice &device,
                     VkSurfaceKHR        surface,
                     VkExtent2D          &requiredExtent,
                     SwapChain           &swapChain);