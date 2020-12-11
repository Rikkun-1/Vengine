#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>
#include <cstring>
#include <set>

/*
* ��������� �� ���� ��������� �����, ��������������� ��������� ���� ��������� � ��������� ��� ��� 
* ��������� ���� ��������������
* � ������ ���� ��� ��������� ���� �������������� ���������� true, ����� false
*/
bool checkValidationLayerSupport(const std::vector<const char *> &requiredLayers);

/* 
* ��������� �� ���� ��������� �� ���������, �������� ���������� � ����������� � ������� DebugMessenger
* � ��������� ������� � ��� ����� ��������� ������ ���� � �������� �� �������� ��������.
* ����� �������� createInfo ��� �� �������� ������������ ������� debugCallback ���
* ������� ��������� ������.
*/
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

/*
* ������� ��������� ������ ������� �������� ��������� �� ����� ��������� � ������� �� � �������,
* ���������� ��� � ��������
*/
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT     messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT            messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void                                       *pUserData);

/*
* ������� DebugMessenger �������� ����� � ��������� VkDebugUtilsMessengerCreateInfoEXT
* � ��������� ��� � ���� instance
*/
VkResult CreateDebugUtilsMessengerEXT(VkInstance                               instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks              *pAllocator,
                                      VkDebugUtilsMessengerEXT                 *pDebugMessenger);

/*
* ���������� DebugMessenger ������� ��� �����-�� ������ ��� ����� instance
*/
void DestroyDebugUtilsMessengerEXT(VkInstance                  instance,
                                   VkDebugUtilsMessengerEXT    debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);


VkDebugUtilsMessengerEXT setupDebugMessenger(VkInstance instance);