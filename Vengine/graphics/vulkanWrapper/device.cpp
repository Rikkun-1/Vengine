#include "device.h"

#include <stdexcept>
#include <set>

#include "swapChain.h"

/*
* Возвращает семейства очередей этого физического устройства, 
* которые удовлетворяет всем требованиям
*/
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice,
                                      VkSurfaceKHR     surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for(auto &queueFamily : queueFamilies)
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

static bool checkDeviceExtensionsSupport(VkPhysicalDevice                physicalDevice,
                                         const std::vector<const char *> &requiredExtensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());


    std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

    for(auto &extension : availableExtensions)
    {
        requiredExtensionsSet.erase(extension.extensionName);
    }

    if(!requiredExtensionsSet.empty())
    {
        std::cout << "\nRequested but not supported device extensions: \n";
        for(auto &extension : requiredExtensionsSet)
        {
            std::cout << extension << '\n';
        }
    }

    return requiredExtensionsSet.empty();
}

/*
* Проверяет что физическое устройство удовлетворяет всем требованиям
*/
static bool isDeviceSuitable(VkPhysicalDevice                physicalDevice,
                             VkSurfaceKHR                    surface,
                             const std::vector<const char *> &requiredExtensions)
        {
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures   deviceFeatures;

            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
            vkGetPhysicalDeviceFeatures  (physicalDevice, &deviceFeatures);

            bool extensionsSupported = checkDeviceExtensionsSupport(physicalDevice, requiredExtensions);

            bool swapChainAdequate = false;
            if(extensionsSupported)
            {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

            return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                   deviceFeatures.geometryShader                                       &&
                   extensionsSupported                                                 &&
                   swapChainAdequate                                                   &&
                   indices.has_value()                                                 &&
                   supportedFeatures.samplerAnisotropy;
        }

static void setupQueueFamilies(const std::set<uint32_t>             &uniqueQueueFamilies,
                               std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos)
{
    float queuePriority = 1.0f;

    for(uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }
}

static void setupDeviceFetures(VkPhysicalDeviceFeatures  &deviceFeatures)
{
    deviceFeatures.samplerAnisotropy = VK_TRUE;
}

VkPhysicalDevice pickPhysicalDevice(VkInstance                      instance,
                                    VkSurfaceKHR                    surface,
                                    const std::vector<const char *> &requiredExtenisons)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if(deviceCount == 0)
        throw std::runtime_error("failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for(auto &device : devices)
    {
        if(isDeviceSuitable(device, surface, requiredExtenisons))
            return device;
    }

    // Если не удалось подобрать подходящее устройство
    throw std::runtime_error("failed to find a suitable GPU!");
}


LogicalDevice createLogicalDevice(VkInstance                      instance,
                                  VkPhysicalDevice                physicalDevice,
                                  VkSurfaceKHR                    surface,
                                  const std::vector<const char *> &requiredExtensions)
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), 
                                               indices.presentFamily.value() };
    
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    setupQueueFamilies(uniqueQueueFamilies,
                       queueCreateInfos);
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    setupDeviceFetures(deviceFeatures);


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    
    LogicalDevice device;
    device.familyIndices = indices;
    if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device.handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");

    vkGetDeviceQueue(device.handle, indices.graphicsFamily.value(), 0, &device.graphicsQueue);
    vkGetDeviceQueue(device.handle, indices.presentFamily.value(),  0, &device.presentQueue);

    return device;
}