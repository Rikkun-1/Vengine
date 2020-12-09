#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb_image.h"

void createImage(VkPhysicalDevice      physicalDevice,
                 VkDevice              logicalDevice,
                 uint32_t              width,
                 uint32_t              height,
                 VkFormat              format,
                 VkImageTiling         tiling,
                 VkImageUsageFlags     usage,
                 VkMemoryPropertyFlags properties,
                 VkImage               &image,
                 VkDeviceMemory        &imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = format;
    imageInfo.tiling        = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = usage;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice,
                                               memRequirements.memoryTypeBits,
                                               properties);

    if(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");

    vkBindImageMemory(logicalDevice, image, imageMemory, 0);
}

void transitionImageLayout(VkDevice      logicalDevice,
                           VkCommandPool commandPool,
                           VkQueue       graphicsQueue,
                           VkImage       image,
                           VkFormat      format,
                           VkImageLayout oldLayout,
                           VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice,
                                                            commandPool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
       newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
            newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if(hasStencilComponent(format))
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer,
                          logicalDevice,
                          commandPool,
                          graphicsQueue);
}

void copyBufferToImage(VkDevice      logicalDevice,
                       VkCommandPool commandPool,
                       VkQueue       graphicsQueue,
                       VkBuffer      buffer,
                       VkImage       image,
                       uint32_t      width,
                       uint32_t      height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice,
                                                            commandPool);

    VkBufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandBuffer,
                          logicalDevice,
                          commandPool,
                          graphicsQueue);
}

void createTextureImage(std::string      path,
                        VkPhysicalDevice physicalDevice,
                        VkDevice         logicalDevice,
                        VkCommandPool    commandPool,
                        VkQueue          graphicsQueue,
                        VkImage          &textureImage,
                        VkDeviceMemory   &textureImageMemory)
{
    int texWidth,
        texHeight,
        texChannels;

    stbi_uc *pixels = stbi_load(path.c_str(),
                                &texWidth,
                                &texHeight,
                                &texChannels,
                                STBI_rgb_alpha);

    // последний аргумент - кол-во байт на пиксель. 4 в случае с форматом STBI_rgb_alpha;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if(!pixels)
        throw std::runtime_error("failed to load texture image!");

    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(physicalDevice,
                 logicalDevice,
                 imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(physicalDevice,
                logicalDevice,
                texWidth,
                texHeight,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                textureImage,
                textureImageMemory);

    transitionImageLayout(logicalDevice,
                          commandPool,
                          graphicsQueue,
                          textureImage,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(logicalDevice,
                      commandPool,
                      graphicsQueue,
                      stagingBuffer,
                      textureImage,
                      static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));

    transitionImageLayout(logicalDevice,
                          commandPool,
                          graphicsQueue,
                          textureImage,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

VkImageView createImageView(VkDevice           logicalDevice,
                            VkImage            image, 
                            VkFormat           format,
                            VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image    = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format   = format;
    viewInfo.subresourceRange.aspectMask     = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    VkImageView imageView;
    if(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");

    return imageView;
}

void createTextureImageView(VkDevice    logicalDevice,
                            VkImage     textureImage,
                            VkImageView &textureImageView)
{
    textureImageView = createImageView(logicalDevice, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void createTextureSampler(VkPhysicalDevice physicalDevice,
                          VkDevice         logicalDevice,
                          VkSampler        &textureSampler)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy    = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp     = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if(vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture sampler!");
}

void createDepthResources(VkPhysicalDevice physicalDevice,
                          VkDevice         logicalDevice,
                          VkCommandPool    commandPool,
                          VkQueue          graphicsQueue,
                          VkExtent2D       swapChainExtent,
                          VkImage          &depthImage,
                          VkDeviceMemory   &depthImageMemory,
                          VkImageView      &depthImageView)
{
    VkFormat depthFormat = findDepthFormat(physicalDevice);

    createImage(physicalDevice,
                logicalDevice,
                swapChainExtent.width, 
                swapChainExtent.height, 
                depthFormat, 
                VK_IMAGE_TILING_OPTIMAL, 
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                depthImage, 
                depthImageMemory);

    depthImageView = createImageView(logicalDevice, 
                                     depthImage, 
                                     depthFormat, 
                                     VK_IMAGE_ASPECT_DEPTH_BIT);
    
    transitionImageLayout(logicalDevice,
                          commandPool,
                          graphicsQueue,
                          depthImage, 
                          depthFormat, 
                          VK_IMAGE_LAYOUT_UNDEFINED, 
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat findSupportedFormat(VkPhysicalDevice            physicalDevice,
                             const std::vector<VkFormat> &candidates, 
                             VkImageTiling               tiling, 
                             VkFormatFeatureFlags        features)
{
    for(VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if(tiling == VK_IMAGE_TILING_LINEAR &&
           (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if(tiling == VK_IMAGE_TILING_OPTIMAL &&
                (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice)
{
    return findSupportedFormat(physicalDevice,
                               {
                                   VK_FORMAT_D32_SFLOAT, 
                                   VK_FORMAT_D32_SFLOAT_S8_UINT, 
                                   VK_FORMAT_D24_UNORM_S8_UINT
                               },
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || 
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}