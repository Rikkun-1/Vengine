#include "setupDevices.h"

bool isDeviceSuitable(const VkPhysicalDevice &physicalDevice,
                      const VkSurfaceKHR     &surface,
                      const std::vector<const char *> &requiredExtensions)
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures   deviceFeatures;

    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures  (physicalDevice, &deviceFeatures);

    bool extensionsSupported = checkDeviceExtensionsSupport(physicalDevice, requiredExtensions);

    //bool swapChainAdequate = false;
    if(extensionsSupported)
    {
        //SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        //swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader                                       &&
           extensionsSupported                                                 &&
           //swapChainAdequate                                                 &&
           indices.has_value();
}

bool checkDeviceExtensionsSupport(const VkPhysicalDevice &physicalDevice,
                                  const std::vector<const char *> &requiredExtensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());


    std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

    for(const auto &extension : availableExtensions)
    {
        requiredExtensionsSet.erase(extension.extensionName);
    }

    if(!requiredExtensionsSet.empty())
    {
        std::cout << "\nRequested but not supported device extensions: \n";
        for(const auto &extension : requiredExtensionsSet)
            std::cout << extension << '\n';
    }

    return requiredExtensionsSet.empty();
}


QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &physicalDevice,
                                     const VkSurfaceKHR     &surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());


    int i = 0;
    for(const auto &queueFamily : queueFamilies)
    {
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

        if(presentSupport)
            indices.presentFamily = i;

        if(indices.has_value())
            break;

        i++;
    }
    return indices;
}