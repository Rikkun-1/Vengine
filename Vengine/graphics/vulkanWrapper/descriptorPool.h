#pragma once

#include <array>

#include "device.h"

VkDescriptorPool createDescriptorPool(const LogicalDevice  &device,
                                      uint32_t             size);