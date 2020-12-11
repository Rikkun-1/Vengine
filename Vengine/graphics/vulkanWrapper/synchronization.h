#pragma once

#include "device.h"

void createSyncObjects(VkDevice                   logicalDevice,
                       int                        MAX_FRAMES_IN_FLIGHT,
                       const std::vector<VkImage> &swapChainImages,
                       std::vector<VkSemaphore>   &imageAvailableSemaphores,
                       std::vector<VkSemaphore>   &renderFinishedSemaphores,
                       std::vector<VkFence>       &inFlightFences,
                       std::vector<VkFence>       &imagesInFlight);
