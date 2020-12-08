#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>

#include "buffer.h"

void createImage(VkPhysicalDevice      physicalDevice,
                 VkDevice              logicalDevice,
                 uint32_t              width,
                 uint32_t              height,
                 VkFormat              format,
                 VkImageTiling         tiling,
                 VkImageUsageFlags     usage,
                 VkMemoryPropertyFlags properties,
                 VkImage               &image,
                 VkDeviceMemory        &imageMemory);

void transitionImageLayout(VkDevice      logicalDevice,
                           VkCommandPool commandPool,
                           VkQueue       graphicsQueue,
                           VkImage       image,
                           VkFormat      format,
                           VkImageLayout oldLayout,
                           VkImageLayout newLayout);

void copyBufferToImage(VkDevice      logicalDevice,
                       VkCommandPool commandPool,
                       VkQueue       graphicsQueue,
                       VkBuffer      buffer,
                       VkImage       image,
                       uint32_t      width,
                       uint32_t      height);

void createTextureImage(VkPhysicalDevice physicalDevice,
                        VkDevice         logicalDevice,
                        VkCommandPool    commandPool,
                        VkQueue          graphicsQueue,
                        VkImage          &textureImage,
                        VkDeviceMemory   &textureImageMemory);

VkImageView createImageView(VkDevice logicalDevice,
                            VkImage  image, 
                            VkFormat format);

void createTextureImageView(VkDevice    logicalDevice,
                            VkImage     textureImage,
                            VkImageView &textureImageView);


void createTextureSampler(VkPhysicalDevice physicalDevice,
                          VkDevice         logicalDevice,
                          VkSampler        &textureSampler);