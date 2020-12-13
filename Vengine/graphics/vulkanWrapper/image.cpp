#include "image.h"

#include <stdexcept>

#include "commandBuffer.h"
#include "transitionImageLayout.h"

///////////////////////// STATIC BEG //////////////////////////////

static void setupImageCopyRegion(const VkExtent3D   &extent,
                                 VkBufferImageCopy  &region)
{
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        extent.width,
        extent.height,
        extent.depth
    };
}

static VkFormat findSupportedFormat(VkPhysicalDevice            physicalDevice,
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

///////////////////////// STATIC END //////////////////////////////

///////////////////////// IMAGE BEG //////////////////////////////

Image::Image()
{
    device   = VK_NULL_HANDLE;
    handle   = VK_NULL_HANDLE;
    memory   = VK_NULL_HANDLE;
    resetImageInfo();
}

Image::Image(const LogicalDevice *device) : Buffer(device)
{
    resetImageInfo();
}

void Image::resetImageInfo()
{
    size     = 0;
    width    = 0;
    height   = 0;
}

void Image::createImage(const VkExtent3D         &extent,
                              VkFormat           format,
                              VkImageTiling      tiling,
                              VkImageUsageFlags  usage)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth  = extent.depth;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = format;
    imageInfo.tiling        = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = usage;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateImage(this->device->handle, &imageInfo, nullptr, &this->handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");
}

void Image::create(const VkExtent3D      &extent,
                   VkFormat              format,
                   VkImageTiling         tiling,
                   VkImageUsageFlags     usage,
                   VkMemoryPropertyFlags properties)
{
    alive = true;

    width    = extent.width;
    height   = extent.height;

    createImage(extent, format, tiling, usage);
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(this->device->handle, this->handle, &memRequirements);
    allocateMemory(memRequirements, properties);
    vkBindImageMemory(this->device->handle, this->handle, this->memory, 0);
}

void Image::destroy()
{
    if(alive)
    {
         vkDestroyImage(device->handle, handle, nullptr);
         Buffer::destroy();
         alive = false;
         resetImageInfo();
    }
}

///////////////////////// IMAGE END //////////////////////////////


///////////////////////// PUBLIC BEG //////////////////////////////

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


void copyBufferToImage(CommandPool    &commandPool,
                       VkExtent3D     &extent,
                       Buffer         &buffer,
                       Image          &image)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

    VkBufferImageCopy region{};
    setupImageCopyRegion(extent, region);

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer.handle,
        image.handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandPool,
                          commandBuffer);
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


void createDepthResources(CommandPool      &commandPool,
                          VkExtent2D       &swapChainExtent,
                          Image            &depthImage,
                          VkImageView      &depthImageView)
{
    VkFormat depthFormat = findDepthFormat(commandPool.device->physicalDevice);

    VkExtent3D depthImageExtent{
        swapChainExtent.width,
        swapChainExtent.height,
        1
    };

    depthImage.setDevice(commandPool.device);
    depthImage.create(depthImageExtent,
                      depthFormat, 
                      VK_IMAGE_TILING_OPTIMAL, 
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


    depthImageView = createImageView(commandPool.device->handle, 
                                     depthImage.handle, 
                                     depthFormat, 
                                     VK_IMAGE_ASPECT_DEPTH_BIT);
    
    transitionImageLayout(commandPool,
                          depthImage, 
                          depthFormat, 
                          VK_IMAGE_LAYOUT_UNDEFINED, 
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}


bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || 
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

///////////////////////// PUBLIC END //////////////////////////////