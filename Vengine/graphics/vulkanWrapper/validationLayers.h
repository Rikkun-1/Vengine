#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>
#include <cstring>
#include <set>

/*
* Принимает на вход множество строк, характеризующее требуемые слои валидации и проверяет что все 
* требуемые слои поддерживаются
* В случае если все требуемые слои поддерживаются возвращает true, иначе false
*/
bool checkValidationLayerSupport(const std::vector<const char *> &requiredLayers);

/* 
* Принимает на вход указатель на структуру, хранящую информацию о создаваемом в будущем DebugMessenger
* и заполняет флагами о том какие сообщения какого типа и важности мы намерены получать.
* Также сообщает createInfo что мы намерены использовать функцию debugCallback как
* функцию обратного вызова.
*/
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

/*
* Функция обратного вызова которая получает сообщения из слоев валидации и выводит их в консоль,
* подписывая тип и важность
*/
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT     messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT            messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void                                       *pUserData);

/*
* Создает DebugMessenger согласно полям в структуре VkDebugUtilsMessengerCreateInfoEXT
* и связывает его с этим instance
*/
VkResult CreateDebugUtilsMessengerEXT(VkInstance                               instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks              *pAllocator,
                                      VkDebugUtilsMessengerEXT                 *pDebugMessenger);

/*
* Уничтожает DebugMessenger который был когда-то создан для этого instance
*/
void DestroyDebugUtilsMessengerEXT(VkInstance                  instance,
                                   VkDebugUtilsMessengerEXT    debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);


VkDebugUtilsMessengerEXT setupDebugMessenger(VkInstance instance);