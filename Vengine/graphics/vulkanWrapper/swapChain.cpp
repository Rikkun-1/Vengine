#include "swapChain.h"

#include "image.h"

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice,
                                              VkSurfaceKHR     surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if(formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if(presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for(auto &availableFormat : availableFormats)
    {
        if(availableFormat.format     == VK_FORMAT_B8G8R8A8_SRGB &&
           availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for(auto &availablePresentMode : availablePresentModes)
    {
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
    }

    return VK_PRESENT_MODE_FIFO_KHR; // Только этот режим гарантированно поддерживается
}

static VkExtent2D chooseSwapExtent(VkExtent2D                     requiredExtent,
                                   const VkSurfaceCapabilitiesKHR &capabilities)
{
    if(capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(requiredExtent.width),
            static_cast<uint32_t>(requiredExtent.height)
        };

        actualExtent.width  = std::max(capabilities.minImageExtent.width,  std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

static void setupSharingMode(VkPhysicalDevice          physicalDevice,
                             VkSurfaceKHR              surface,
                             VkSwapchainCreateInfoKHR  &createInfo)
{
    QueueFamilyIndices indices    = findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if(indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices   = nullptr; // Optional
    }
}

static uint32_t getMinImageCount(SwapChainSupportDetails &swapChainSupport)
{
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    //maxImageCount равный нулю означает что здесь нет ограничения на количество изображений
    if(swapChainSupport.capabilities.maxImageCount > 0  &&
       imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
}


void createImageViews(const LogicalDevice &device,
                      SwapChain           swapChain)
{
    swapChain.imageViews.resize(swapChain.images.size());

    for(uint32_t i = 0; i < swapChain.images.size(); i++)
    {
        swapChain.imageViews[i] = createImageView(device.handle,
                                                  swapChain.images[i],
                                                  swapChain.imageFormat,
                                                  VK_IMAGE_ASPECT_COLOR_BIT);
    }
}


void createFramebuffers(VkDevice          logicalDevice,
                        VkRenderPass      renderPass,
                        SwapChain         swapChain,
                        VkImageView       depthImageView)
{
    swapChain.frameBuffers.resize(swapChain.imageViews.size());

    for(size_t i = 0; i < swapChain.imageViews.size(); i++)
    {
        // вложения буфера. В нашем случае каждый буфер хранит в себе одно изображение из 
        // swap chain и общую для всех изображений карту глубины
        std::array<VkImageView, 2> attachments = {
            swapChain.imageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments    = attachments.data();
        framebufferInfo.width           = swapChain.extent.width;
        framebufferInfo.height          = swapChain.extent.height;
        framebufferInfo.layers          = 1;

        if(vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChain.frameBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}


void createSwapChain(const LogicalDevice &device,
                     VkSurfaceKHR        surface,
                     VkExtent2D          &requiredExtent,
                     SwapChain           &swapChain)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device.physicalDevice, surface);
    VkSurfaceFormatKHR      surfaceFormat    = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR        presentMode      = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D              actualExtent     = chooseSwapExtent(requiredExtent, swapChainSupport.capabilities);
    uint32_t                imageCount       = getMinImageCount(swapChainSupport);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = actualExtent;
    createInfo.imageArrayLayers = 1; // Количество слоев на которых будут располагаться

    // данная цепочка показа предназначается непосредственно для рисования прямо в нее
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    setupSharingMode(device.physicalDevice, surface, createInfo);

    createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;

    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device.handle, &createInfo, nullptr, &swapChain.handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");


    vkGetSwapchainImagesKHR(device.handle, swapChain.handle, &imageCount, nullptr);

    swapChain.images.resize(imageCount);
    vkGetSwapchainImagesKHR(device.handle, swapChain.handle, &imageCount, swapChain.images.data());

    createImageViews(device, swapChain);

    swapChain.imageFormat = surfaceFormat.format;
    swapChain.extent      = actualExtent;
}
