#pragma once

#include "image.h"
#include "transitionImageLayout.h"

#include "texture.h"

static void setupSamplerCreateInfo(const LogicalDevice        &device,
                                   VkSamplerCreateInfo  &samplerInfo);


void createTextureImage(void               *pixels,
                        int                 textureChannels,
                        VkExtent3D         &textureExtent,
                        CommandPool        &commandPool,
                        Image              &textureImage);


void createTextureImageView(const LogicalDevice  &device,
                            Image          &textureImage,
                            VkImageView    &textureImageView);


void createTextureSampler(const LogicalDevice    &device,
                          VkSampler        &textureSampler);
