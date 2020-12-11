#include "transitionImageLayout.h"

#include "commandBuffer.h"
#include "image.h"

static enum class TransitionType
{
    WRITE_OPTIMAL         = 0,
    SHADER_READ_OPTIMAL   = 1,
    DEPTH_STENCIL_OPTIMAL = 2
};


static struct ImageTransitionInfo
{
    VkImageMemoryBarrier barrier{};

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    void setupForTransition(TransitionType transitionType,
                            VkFormat       imageFormat);

private:
    void setupForDataWriting();
    void setupForShaderReading();
    void setupForDepthStencil(VkFormat  format);
};

void ImageTransitionInfo::setupForTransition(TransitionType transitionType, 
                                             VkFormat       imageFormat)
{
    switch(transitionType)
    {
        case TransitionType::WRITE_OPTIMAL:

            setupForDataWriting(); break;

        case TransitionType::SHADER_READ_OPTIMAL:

            setupForShaderReading(); break;

        case TransitionType::DEPTH_STENCIL_OPTIMAL:

            setupForDepthStencil(imageFormat); break;

        default:
            throw std::invalid_argument("unsupported layout transition!");
    }
}

void ImageTransitionInfo::setupForDataWriting()
{
     barrier.srcAccessMask = 0;
     barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

     sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
     destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
}

void ImageTransitionInfo::setupForShaderReading()
{
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
}

void ImageTransitionInfo::setupForDepthStencil(VkFormat  format)
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


static TransitionType getTransitionType(VkImageLayout  oldLayout,
                                        VkImageLayout  newLayout)
{
    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
       newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        return TransitionType::WRITE_OPTIMAL;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        return TransitionType::SHADER_READ_OPTIMAL;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
            newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        return TransitionType::DEPTH_STENCIL_OPTIMAL;
    }
}


static void setupImageMemoryBarrier(VkImageLayout          oldLayout,
                                    VkImageLayout          newLayout,
                                    VkImage                image,
                                    VkImageMemoryBarrier  &barrier)
{
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
}


void transitionImageLayout(CommandPool    &commandPool,
                           Image          &image,
                           VkFormat       format,
                           VkImageLayout  oldLayout,
                           VkImageLayout  newLayout)
{
    VkCommandBuffer commandBuffer;
    commandBuffer = beginSingleTimeCommands(commandPool);
    
    TransitionType transitionType = getTransitionType(oldLayout, newLayout);

    ImageTransitionInfo  transitionInfo{};
    transitionInfo.setupForTransition(transitionType, format);
    
    setupImageMemoryBarrier(oldLayout, newLayout, image.handle, transitionInfo.barrier);

    vkCmdPipelineBarrier(
        commandBuffer,
        transitionInfo.sourceStage, transitionInfo.destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &transitionInfo.barrier
    );

    endSingleTimeCommands(commandPool,
                          commandBuffer);
}