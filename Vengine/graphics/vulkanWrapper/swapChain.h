#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "device.h"

struct SwapChain
{
    LogicalDevice              *device;
    VkSwapchainKHR              handle;
    std::vector<VkImage>        images;
    std::vector<VkImageView>    imageViews;
    std::vector<VkFramebuffer>  frameBuffers;
    VkFormat                    imageFormat;
    VkExtent2D                  extent;

    void create(const LogicalDevice &device,
                VkSurfaceKHR        surface,
                VkExtent2D          &requiredExtent);

    void destroy();
    
    void createFrameBuffers(VkRenderPass  renderPass,
                            VkImageView   depthImageView);
private:
    void createImageViews();
};


struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};


SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice,
                                              VkSurfaceKHR     surface);




