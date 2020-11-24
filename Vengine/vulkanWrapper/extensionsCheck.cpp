#include "extensionsCheck.h"

bool checkExtensionsSupport(const std::vector<const char *> &requiredExtensions)
{
	uint32_t supportedExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);

	std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

	std::cout << "\nSupported extensions: \n";
	for(const auto &extension : supportedExtensions)
	{
		std::cout << '\t' << extension.extensionName << '\n';
	}

	std::cout << "\nRequered extensions: \n";
	for(const auto &extension : requiredExtensions)
	{
		std::cout << '\t' << extension << '\n';
	}

	std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

	for(const auto &extension : supportedExtensions)
	{
		requiredExtensionsSet.erase(extension.extensionName);
	}

	if(!requiredExtensionsSet.empty())
	{
		std::cout << "\nUnsupported extensions: \n";
		for(const auto &extension : requiredExtensionsSet)
		{
			std::cout << '\t' << extension << '\n';
		}
	}

	return requiredExtensionsSet.empty();
}


std::vector<const char *> getRequiredExtensions(bool enableValidationLayers)
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char *> requeredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if(enableValidationLayers)
	{
		requeredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return requeredExtensions;
}