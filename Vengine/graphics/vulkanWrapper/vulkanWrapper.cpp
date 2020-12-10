#include "vulkanWrapper.h"

///////////////////////////////////////////////////////////

void createSyncObjects(VkDevice                   logicalDevice,
                       int                        MAX_FRAMES_IN_FLIGHT,
                       const std::vector<VkImage> &swapChainImages,
                       std::vector<VkSemaphore>   &imageAvailableSemaphores,
                       std::vector<VkSemaphore>   &renderFinishedSemaphores,
                       std::vector<VkFence>       &inFlightFences,
                       std::vector<VkFence>       &imagesInFlight)
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

    // согласно текущему состоянию Vulkan API для создания семафора более не требуется никакой
    // информации кроме sType
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // говорим что при создании наш забор должен быть сразу в сигнальном состоянии
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
           vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
           vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}



