#include "instance.h"

#include <stdexcept>
#include <iostream>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
    
///////////////////////// PUBLIC BEG //////////////////////////////

/*
* Проверяет что все требуемые расширения поддеживаются 
* Выводит в консоль поддерживаемые расширения
* Выводит в консоль требуемые расширения
* Выводит в консоль те расширения которые требуются, но не поддерживаются
* Возвращает true если все требуемые расширения поддерживаются и false в ином случае
*/
bool checkExtensionsSupport(const std::vector<const char *> &requiredExtensions)
{
    uint32_t supportedExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);

    std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

    std::cout << "\nSupported extensions: \n";
    for(auto &extension : supportedExtensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }

    std::cout << "\nRequered extensions: \n";
    for(auto &extension : requiredExtensions)
    {
        std::cout << '\t' << extension << '\n';
    }

    std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

    for(auto &extension : supportedExtensions)
    {
        requiredExtensionsSet.erase(extension.extensionName);
    }

    if(!requiredExtensionsSet.empty())
    {
        std::cout << "\nUnsupported extensions: \n";
        for(auto &extension : requiredExtensionsSet)
        {
            std::cout << '\t' << extension << '\n';
        }
    }

    return requiredExtensionsSet.empty();
}

/*
* Возвращает список расширений, требуемых для glfw и слоев валидации, если они включены
*/
std::vector<const char *> getRequiredExtensions(bool enableValidationLayers)
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> requeredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(enableValidationLayers)
        requeredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return requeredExtensions;
}

#include <iostream>
VkInstance createInstance(const std::vector<const char *> &validationLayers,
                          const std::vector<const char *> &instanceExtensions)
{
    if(enableValidationLayers && !checkValidationLayerSupport(validationLayers))
        throw std::runtime_error("validation layers requested, but not available");


    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "3D graphics engine";
    appInfo.pEngineName        = "Vengine";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;


    VkInstanceCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;


    auto extensions = getRequiredExtensions(enableValidationLayers);

    // Прибавляем к расширениям требуемым для glfw и слоев валидации расширения, те
    // которые требует пользователь
    extensions.insert(extensions.end(), 
                      instanceExtensions.begin(), 
                      instanceExtensions.end());

    if(!checkExtensionsSupport(extensions))
        std::runtime_error("UNSUPPORTED EXTENSION");
    

    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if(enableValidationLayers)
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    VkInstance instance;
    if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance!");

    return instance;
}

///////////////////////// PUBLIC END //////////////////////////////