#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <set>

#include "queueFamilyIndices.h"
#include "setupSwapchain.h"

/*
* ��������� ��� ���������� ���������� ������������� ���� �����������
*/
bool isDeviceSuitable(const VkPhysicalDevice &physicalDevice,
					  const VkSurfaceKHR	 &surface, 
					  const std::vector<const char *> &requiredExtensions);

bool checkDeviceExtensionsSupport(const VkPhysicalDevice &physicalDevice,
								  const std::vector<const char *> &requiredExtensions);

/*
* ���������� ��������� �������� ����� ����������� ����������, 
* ������� ������������� ���� �����������
*/
QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &physicalDevice,
									 const VkSurfaceKHR	    &surface);

