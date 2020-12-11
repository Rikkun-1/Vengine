#include "texture.h"

static void setupSamplerCreateInfo(LogicalDevice        &device,
                                   VkSamplerCreateInfo  &samplerInfo)
{
    samplerInfo.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device.physicalDevice, &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy    = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp     = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod     = 0.0f;
    samplerInfo.maxLod     = 0.0f;
}


void createTextureImage(void               *pixels,
                        int                 textureChannels,
                        VkExtent3D         &textureExtent,
                        CommandPool        &commandPool,
                        Image              &textureImage)
{
    textureImage.size = textureExtent.width * textureExtent.height * textureChannels;

    Buffer stagingBuffer(commandPool.device);
    
    createStagingBuffer(textureImage.size, stagingBuffer);

    stagingBuffer.mapMemory(textureImage.size, 
                            pixels);

    textureImage.create(textureExtent,
                        VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                        VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


    transitionImageLayout(commandPool,
                          textureImage.handle,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);


    copyBufferToImage(commandPool,
                      textureExtent,
                      stagingBuffer,
                      textureImage);


    transitionImageLayout(commandPool,
                          textureImage.handle,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}


void createTextureImageView(LogicalDevice  &device,
                            Image          &textureImage,
                            VkImageView    &textureImageView)
{
    textureImageView = createImageView(device.handle, 
                                       textureImage.handle, 
                                       VK_FORMAT_R8G8B8A8_SRGB, 
                                       VK_IMAGE_ASPECT_COLOR_BIT);
}

void createTextureSampler(LogicalDevice    &device,
                          VkSampler        &textureSampler)
{
    VkSamplerCreateInfo samplerInfo{};
    setupSamplerCreateInfo(device, samplerInfo);

    if(vkCreateSampler(device.handle, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture sampler!");
}
