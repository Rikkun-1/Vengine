#pragma once

#include "device.h"
#include "swapChain.h"

void createSyncObjects(const LogicalDevice        &device,
                       int                        MAX_FRAMES_IN_FLIGHT,
                       const Swapchain            &swapChain,
                       std::vector<VkSemaphore>   &imageAvailableSemaphores,
                       std::vector<VkSemaphore>   &renderFinishedSemaphores,
                       std::vector<VkFence>       &inFlightFences,
                       std::vector<VkFence>       &imagesInFlight);
