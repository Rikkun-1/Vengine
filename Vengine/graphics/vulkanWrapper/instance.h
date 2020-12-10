#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "validationLayers.h"

VkInstance createInstance(const std::vector<const char *> &validationLayers,
                          const std::vector<const char *> &instanceExtensions);

VkDebugUtilsMessengerEXT setupDebugMessenger(VkInstance instance)