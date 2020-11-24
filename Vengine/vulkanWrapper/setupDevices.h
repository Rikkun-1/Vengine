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
bool isDeviceSuitable(const VkPhysicalDevice &physicalDevice,
					  const VkSurfaceKHR	 &surface, 
					  const std::vector<const char *> &requiredExtensions);

bool checkDeviceExtensionsSupport(const VkPhysicalDevice &physicalDevice,
								  const std::vector<const char *> &requiredExtensions);

/*
* Возвращает семейства очередей этого физического устройства, 
* которые удовлетворяет всем требованиям
*/
QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &physicalDevice,
									 const VkSurfaceKHR	    &surface);

