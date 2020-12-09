#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <set>

#include "queueFamilyIndices.h"
#include "setupSwapchain.h"

/*
* Проверяет что физическое устройство удовлетворяет всем требованиям
*/
bool isDeviceSuitable(VkPhysicalDevice                physicalDevice,
                      VkSurfaceKHR                    surface, 
                      const std::vector<const char *> &requiredExtensions);

bool checkDeviceExtensionsSupport(VkPhysicalDevice               physicalDevice,
                                  const std::vector<const char *> &requiredExtensions);

/*
* Возвращает семейства очередей этого физического устройства, 
* которые удовлетворяет всем требованиям
*/
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice,
                                     VkSurfaceKHR     surface);

