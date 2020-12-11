#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>

#include "buffer.h"
#include "transitionImageLayout.h"

struct Image: public Buffer
{
    VkImage  handle;
    int      width;
    int      height;
    int      channels;

    Image(const LogicalDevice *device);

    void create(const VkExtent3D      &extent,
                VkFormat              format,
                VkImageTiling         tiling,
                VkImageUsageFlags     usage,
                VkMemoryPropertyFlags properties);
private:
    void createImage(const VkExtent3D      &extent,
                     VkFormat              format,
                     VkImageTiling         tiling,
                     VkImageUsageFlags     usage);
};

void copyBufferToImage(CommandPool    &commandPool,
                       VkExtent3D     &extent,
                       Buffer         &buffer,
                       Image          &image);


VkImageView createImageView(VkDevice           logicalDevice,
                            Image              &image, 
                            VkFormat           format,
                            VkImageAspectFlags aspectFlags);


void createDepthResources(CommandPool      &commandPool,
                          VkExtent2D       &swapChainExtent,
                          Image            &depthImage,
                          VkImageView      &depthImageView);


bool hasStencilComponent(VkFormat format);