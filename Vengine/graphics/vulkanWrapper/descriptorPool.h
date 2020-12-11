#pragma once

#include "device.h"

VkDescriptorPool createDescriptorPool(LogicalDevice  &device,
                                      int            size);