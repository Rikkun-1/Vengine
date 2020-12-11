#pragma once

#include "image.h"

#include "texture.h"

static void setupSamplerCreateInfo(LogicalDevice        &device,
                                   VkSamplerCreateInfo  &samplerInfo);


void createTextureImage(void               *pixels,
                        int                 textureChannels,
                        VkExtent3D         &textureExtent,
                        CommandPool        &commandPool,
                        Image              &textureImage);


void createTextureImageView(LogicalDevice  &device,
                            Image          &textureImage,
                            VkImageView    &textureImageView);


void createTextureSampler(LogicalDevice    &device,
                          VkSampler        &textureSampler);
