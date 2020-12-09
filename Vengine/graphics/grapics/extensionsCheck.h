#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <set>

/*
* Проверяет что все требуемые расширения поддеживаются 
* Выводит в консоль поддерживаемые расширения
* Выводит в консоль требуемые расширения
* Выводит в консоль те расширения которые требуются, но не поддерживаются
* Возвращает true если все требуемые расширения поддерживаются и false в ином случае
*/
bool checkExtensionsSupport(const std::vector<const char *> &requiredExtenisons);

/*
* Возвращает список расширений, требуемых для glfw и слоев валидации, если они включены
*/
std::vector<const char *> getRequiredExtensions(bool enableValidationLayers);
