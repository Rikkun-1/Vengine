#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "buffer.h"
#include "commandPool.h"

struct Image: public Buffer
{
    VkImage                handle;

    int                    width;
    int                    height;

    Image();
    Image(const LogicalDevice *device);

    void create(const VkExtent3D      &extent,
                VkFormat              format,
                VkImageTiling         tiling,
                VkImageUsageFlags     usage,
                VkMemoryPropertyFlags properties);

    void destroy();

private:
    void createImage(const VkExtent3D      &extent,
                     VkFormat              format,
                     VkImageTiling         tiling,
                     VkImageUsageFlags     usage);

    void resetImageInfo();
};

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);


void copyBufferToImage(CommandPool    &commandPool,
                       VkExtent3D     &extent,
                       Buffer         &buffer,
                       Image          &image);


VkImageView createImageView(VkDevice           logicalDevice,
                            VkImage            image, 
                            VkFormat           format,
                            VkImageAspectFlags aspectFlags);


void createDepthResources(CommandPool      &commandPool,
                          VkExtent2D       &swapChainExtent,
                          Image            &depthImage,
                          VkImageView      &depthImageView);


bool hasStencilComponent(VkFormat format);