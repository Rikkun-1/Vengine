#include "frameBuffer.h"

#include <stdexcept>

VkFramebuffer createFrameBuffer(const LogicalDevice              &device,
                                VkRenderPass                      renderPass,
                                const VkExtent3D                 &extent,
                                const std::array<VkImageView, 2> &attachments)
{
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass      = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = extent.width;
    framebufferInfo.height          = extent.height;
    framebufferInfo.layers          = extent.depth;
    
    VkFramebuffer frameBuffer;
    if(vkCreateFramebuffer(device.handle, &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create framebuffer!");

    return frameBuffer;
}
