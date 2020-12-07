#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <set>

/*
* ��������� ��� ��� ��������� ���������� ������������� 
* ������� � ������� �������������� ����������
* ������� � ������� ��������� ����������
* ������� � ������� �� ���������� ������� ���������, �� �� ��������������
* ���������� true ���� ��� ��������� ���������� �������������� � false � ���� ������
*/
bool checkExtensionsSupport(const std::vector<const char *> &requiredExtenisons);

/*
* ���������� ������ ����������, ��������� ��� glfw � ����� ���������, ���� ��� ��������
*/
std::vector<const char *> getRequiredExtensions(bool enableValidationLayers);
