#pragma once

#include "image.h"

#include "texture.h"

static void setupSamplerCreateInfo(LogicalDevice        &device,
                                   VkSamplerCreateInfo  &samplerInfo);


void createTextureImage(unsigned char      *pixels,
                        int                 textureChannels,
                        VkExtent3D         &textureExtent,
                        CommandPool        &commandPool,
                        Image              &textureImage);


void createTextureImageView(VkDevice    logicalDevice,
                            Image       textureImage,
                            VkImageView &textureImageView);


void createTextureSampler(LogicalDevice    &device,
                          VkSampler        &textureSampler);
