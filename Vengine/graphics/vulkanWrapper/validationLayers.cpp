#include "validationLayers.h"

#ifdef USE_VALIDATION_LAYERS
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

bool checkValidationLayerSupport(const std::vector<const char *> &requiredLayers)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::set<std::string> requiredLayersSet(requiredLayers.begin(), requiredLayers.end());

    for(auto &layer : availableLayers)
    {
        requiredLayersSet.erase(layer.layerName);
    }

    if(!requiredLayersSet.empty())
    {
        std::cout << "\nRequested but not supported layers: \n";
        for(auto &layer : requiredLayersSet)
        {
            std::cout << layer << '\n';
        }
    }

    return requiredLayersSet.empty();
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                                 //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    | //отключаем неинтересные нам информационные сообщения
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType       = //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT  | 
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    createInfo.pfnUserCallback = debugCallback;

    // Опциональный параметр который позволяет передавать некоторую информацию 
    // в функцию обратного вызова сквозь слои валидации 
    createInfo.pUserData = nullptr; 
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT     messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT            messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void                                       *pUserData)
{
    std::cout << "\nvalidation layer: ";
    std::cout << "\nmessage severenity: ";

    if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)         std::cout << "ERROR";
    else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)  std::cout << "WARNING";
    else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)     std::cout << "INFO";
    else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)  std::cout << "VERBOSE";

    std::cout << "\nmessage type      : ";

    if(messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)          std::cout << "GENERAL";
    else if(messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)  std::cout << "VALIDATION";
    else if(messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) std::cout << "PERFORMANCE";

    std::cout << "\nmessage: " << pCallbackData->pMessage << "\n";

    //если вернуть VK_TRUE, то это будет значить что после обраного вызова необходимо прекратить работу с ошибкой
    //VK_ERROR_VALIDATION_FAILED_EXT 
    return VK_FALSE;
}


VkResult CreateDebugUtilsMessengerEXT(VkInstance                               instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks              *pAllocator,
                                      VkDebugUtilsMessengerEXT                 *pDebugMessenger)
{
    // Так как слои валидации являются частью расширения, то чтобы создать DebugUtilsMessenger
    // необходимо сначала загрузить соответствующую функцию PFN_vkCreateDebugUtilsMessengerEXT
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}


void DestroyDebugUtilsMessengerEXT(VkInstance                  instance,
                                   VkDebugUtilsMessengerEXT    debugMessenger,
                                   const VkAllocationCallbacks *pAllocator)
{
    // Так как слои валидации являются частью расширения, то чтобы уничтожить DebugUtilsMessenger
    // необходимо сначала загрузить соответствующую функцию PFN_vkDestroyDebugUtilsMessengerEXT
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr)
        func(instance, debugMessenger, pAllocator);
}


VkDebugUtilsMessengerEXT setupDebugMessenger(VkInstance instance)
{
    if(!enableValidationLayers) return VK_NULL_HANDLE;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);
    
    VkDebugUtilsMessengerEXT debugMessenger;
    if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("failed to set up debug messenger!");

    return debugMessenger;
}
