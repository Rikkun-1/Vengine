#pragma once

#include <array>

#include "device.h"

VkDescriptorPool createDescriptorPool(const LogicalDevice  &device,
                                      int            size);