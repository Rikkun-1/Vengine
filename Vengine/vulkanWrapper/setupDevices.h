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
bool isDeviceSuitable(VkPhysicalDevice                physicalDevice,
                      VkSurfaceKHR                    surface, 
                      const std::vector<const char *> &requiredExtensions);

bool checkDeviceExtensionsSupport(VkPhysicalDevice               physicalDevice,
                                  const std::vector<const char *> &requiredExtensions);

/*
* ���������� ��������� �������� ����� ����������� ����������, 
* ������� ������������� ���� �����������
*/
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice,
                                     VkSurfaceKHR     surface);

