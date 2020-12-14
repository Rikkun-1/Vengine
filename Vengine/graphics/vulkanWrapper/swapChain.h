#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "device.h"

#include "image.h"
#include "framebuffer.h"

struct Swapchain
{
public:
    LogicalDevice              *device;
    VkSwapchainKHR              handle;
    std::vector<VkImage>        images;
    std::vector<VkImageView>    imageViews;
    std::vector<VkFramebuffer>  frameBuffers;
    VkFormat                    imageFormat;
    VkExtent2D                  extent;

    void create(LogicalDevice  &device,
                VkSurfaceKHR    surface,
                VkExtent2D     &requiredExtent);

    void destroy();
    
    void createFrameBuffers(VkRenderPass  renderPass,
                            VkImageView   depthImageView);
private:
    void createImageViews();
};




