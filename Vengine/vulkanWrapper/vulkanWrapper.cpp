
#include "vulkanWrapper.h"


void createInstance(const std::vector<const char *> &validationLayers,
					const std::vector<const char *> &instanceExtensions,
					VkInstance					    &instance)
{
	if(enableValidationLayers && !checkValidationLayerSupport(validationLayers))
	{
		throw std::runtime_error("validation layers requested, but not available");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType			   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "3D graphics engine";
	appInfo.pEngineName		   = "Vengine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion	   = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion	       = VK_API_VERSION_1_0;


	VkInstanceCreateInfo createInfo{};
	createInfo.sType		    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;


	auto extensions = getRequiredExtensions(enableValidationLayers);

	// ���������� � ����������� ��������� ��� glfw � ����� ��������� ����������, ��
	// ������� ������� ������������
	extensions.insert(extensions.end(), 
					  instanceExtensions.begin(), 
					  instanceExtensions.end());

	if(!checkExtensionsSupport(extensions))
	{
		std::runtime_error("UNSUPPORTED EXTENSION");
	}
	
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

	if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}


void setupDebugMessenger(VkInstance &instance, VkDebugUtilsMessengerEXT &debugMessenger)
{
	if(!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);
	
	if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}


VkPhysicalDevice pickPhysicalDevice(const VkInstance   &instance,
									const VkSurfaceKHR &surface,
									const std::vector<const char *> &requiredExtenisons)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if(deviceCount == 0)
		throw std::runtime_error("failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for(const auto &device : devices)
	{
		if(isDeviceSuitable(device, surface, requiredExtenisons))
		{
			return device;
		}
	}

	// ���� �� ������� ��������� ���������� ����������
	throw std::runtime_error("failed to find a suitable GPU!");
}


void createLogicalDevice(const VkPhysicalDevice &physicalDevice,
						 const VkSurfaceKHR		&surface,
						 const std::vector<const char *> &requiredExtenisons,
						 VkDevice &logicalDevice,
						 VkQueue  &graphicsQueue,
						 VkQueue  &presentQueue)
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	for(uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType			 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount		 = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos    = queueCreateInfos.data();

	createInfo.pEnabledFeatures		= &deviceFeatures;

	createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtenisons.size());
	createInfo.ppEnabledExtensionNames = requiredExtenisons.data();

	if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		throw std::runtime_error("failed to create logical device!");

	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}


void createSwapChain(GLFWwindow				 *pWindow,
					 const VkPhysicalDevice  &physicalDevice,
					 const VkDevice		 	 &logicalDevice,
					 const VkSurfaceKHR	 	 &surface,
					 VkSwapchainKHR			 &swapChain,
					 std::vector<VkImage>	 &swapChainImages,
					 VkFormat				 &swapChainImageFormat,
					 VkExtent2D				 &swapChainExtent)
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
	VkSurfaceFormatKHR		surfaceFormat    = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR		presentMode		 = chooseSwapPresentMode(swapChainSupport.presentModes);
	
	int width;
	int height;
	glfwGetFramebufferSize(pWindow, &width, &height);
	VkExtent2D extent = chooseSwapExtent(width, height, swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	//maxImageCount ������ ���� �������� ��� ����� ��� ����������� �� ���������� �����������
	if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount    = imageCount;
	createInfo.imageFormat      = surfaceFormat.format;
	createInfo.imageColorSpace  = surfaceFormat.colorSpace;
	createInfo.imageExtent      = extent;
	createInfo.imageArrayLayers = 1; // ���������� ����� �� ������� ����� �������������

	// ������ ������� ������ ��������������� ��������������� ��� ��������� ����� � ���
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if(indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode		 = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices   = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode		 = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices   = nullptr; // Optional
	}

	createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode	  = presentMode;
	createInfo.clipped		  = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if(vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void createImageViews(const VkDevice			 &logicalDevice,
					  const VkFormat		     &swapChainImageFormat,
					  const std::vector<VkImage> &swapChainImages,
					  std::vector<VkImageView>	 &swapChainImageViews)
{
	swapChainImageViews.resize(swapChainImages.size());

	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		// ��� ��������� �������� �� ���������� ����� � �����������
		// �� ��������� ��� ������ ����������� ������� �� ������ ����
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if(vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}
///////////////////////////////////////////////////////////

void createRenderPass(const VkDevice	&logicalDevice,
					  const VkFormat	&swapChainImageFormat,
					  VkRenderPass		&renderPass)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format  = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	// ��� ������ � ������� �� �� ����������
	// VK_ATTACHMENT_LOAD_OP_CLEAR - ��������� ����� �� ����������. ����������� ���������� � ������� �������
	// VK_ATTACHMENT_LOAD_OP_LOAD  - �������� ��� ����
	// VK_ATTACHMENT_LOAD_OP_DONT_CARE - ��� �� �����
	colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;

	// ��� ������ ����� ����������
	// VK_ATTACHMENT_STORE_OP_STORE - ��������� ����� ��� ������������ �������������. �������� ��� ������
	// VK_ATTACHMENT_STORE_OP_DONT_CARE - ��� �� ������� ��� ���������� � ����������� ����� ����������
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	// ���� ����� ��� ������ - ���������. �� ��� ���� �� ����������. ��� �� ������� ��� ���������
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// ������ �������� ��������� ������������ ����������� � ������ �������� ���������� ��������.
	// �.�. �� ������� ��� �� ����� ������ � ������������ � ���������� ����� ������� ��� � ��� ����,
	// ������� ����� �������� ����������� ��� ������ ��������
	// initialLayout - ��� ����� ����� ��� ����������� �������������� �����
	// finalLayout - ��� ����� ����� ��� ����������� ����� �������������� ����� ���� ��������
	// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR - ��� ����������� ����� �������� ��� �������� � swap chain
	// �� ����� ����� ����� ��������� ��� ����������� ����� �� ���� ������ � �������� � swap chain
	// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL - �� �������� ������ �������� ����
	// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL - ��� ����������� ����� ���������� ��� �����������
	// ����� ����� ����� ����������� ���������� � ���������� ��������� ��� ������ ��������
	// VK_IMAGE_LAYOUT_UNDEFINED - �� �� ����� � ����� ��������� ���� ����������� �����������
	// ��� ������� ��� ��� ����������� ����� ���� � ������������ ��������� ��� ������ ������
	// ��� ��� �� ������� ��� ��� �� ��� ����� �������� ����������� 
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // ������ attachment �� ������� �� ���������
	// ��� ����� ����� ����� �������������� ���� attachment ��������������� ������
	// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL - ������������� ��� ��������� � ����
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// �������� �������� �������
	VkSubpassDescription subpass{};
	// ��� �������� ������ ������
	// VK_PIPELINE_BIND_POINT_GRAPHICS - ������ ������ �������� � ��������
	// VK_PIPELINE_BIND_POINT_COMPUTE - ������ ������ �������� � ������������
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// ������ attachment'� ��� � ���� ���� �� ������� �� ������ �� ����� ������ � ��������
	// layout(location = 0) out vec4 outColor
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// � ���� ������ ����� ���� �������� ��������� ����������:
	// pInputAttachments: ��� ���������� ����� �������������� ��� ���� ��� ��������
	// pResolveAttachments : ��� ���������� ������������ ��� ���������������
	// pDepthStencilAttachment : ��� ������������ ����� ��� ����� ������ � ���������
	// pPreserveAttachments : ���� ���� �� ������������ �� ����� �������� �������, 
	// �� ��� ������ ������ ���� ������������� � �������������
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType		   = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments    = &colorAttachment;
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpass;

	if(vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}

	VkSubpassDependency dependency{};
	// ������ ���������� ��� �������� �� ����������� �����������
	// VK_SUBPASS_EXTERNAL ��������� ������� ���������� ������� ���������� �� ��� ����� 
	// ������ render pass � ����������� �� ���� ���������� �� � srcSubpass ��� dstSubpass
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

	// ������ ��������� ����������. ��������� � ��� ���� ������������ � ��� ������� ����� 0
	dependency.dstSubpass = 0; 
	//dstSubpass ������ ������ ���� ������ ��� srcSubpass �� ��������� ������������ � ����� ������������

	// ����� �� ��������� �� ���� �� ���� ������ ��� ������ � �� ����� ������ �� ����� ����
	// �� ������� ��� ������ ��� ������ ������ ��������� � ����� �� ����� ��������� 
	// ���������� �� ������
	// ������ ��� ������ �������� �� ����, ��� ������� ������ �������� ������ � ������������� �����������
	dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies   = &dependency;
}

///////////////////////////////////////////////////////////
VkShaderModule createShaderModule(const VkDevice &logicalDevice,
								  const std::vector<char> &code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());

	VkShaderModule shaderModule;
	if(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}


void createGraphicsPipeline(const VkDevice			&logicalDevice,
							const VkExtent2D		&swapChainExtent,
							VkRenderPass			&renderPass,
							VkPipelineLayout		&pipelineLayout,
							VkPipeline				&graphicsPipeline,
							VkDescriptorSetLayout	&descriptorSetLayout)
{
	auto vertShaderCode = readFile("shaders/bin/vert.spv");
	auto fragShaderCode = readFile("shaders/bin/frag.spv");
	// ���� ������� ������ ������ ��� ��� �����������

	VkShaderModule vertShaderModule = createShaderModule(logicalDevice, vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(logicalDevice, fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName  = "main"; // ����� �����

	// ������������ ��������, ������� ��������� ���������� ��������� �������� � ������
	// �������� �� ��� �������� ������ ����� ������ ���� ���������
	vertShaderStageInfo.pSpecializationInfo = nullptr;


	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName  = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


	//////////////////////// VERTEX INPUT STAGE ////////////////////////

	auto bindingDescription	   = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType						    = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount	= 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions		= &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();
	////////////////////////////////////////////////////////////////////


	//////////////////////// INPUT ASSEMBLY STAGE ////////////////////////

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType	   = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//////////////////////////////////////////////////////////////////////


	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width  = (float) swapChainExtent.width;
	viewport.height = (float) swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;

	//////////////////////// VIEWPORT SETUP ////////////////////////

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports    = &viewport;
	viewportState.scissorCount  = 1;
	viewportState.pScissors		= &scissor;
	//������� ��������� viewport ��� scissors ������� ���������
	/////////////////////////////////////////////////////////////////


	//////////////////////// RASTERIZER STAGE ////////////////////////

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; // VK_TRUE ������� ����������

	// ���������� ����� ������������. ���� VK_TRUE, �� 
	// �� framebuffer ������ ������ �� ����� ��������
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	// ���������� ����� ������� ����� �������������� ������ �������
	// � ����� ������ �� ��������� ��� �������
	// VK_POLYGON_MODE_LINE  - ����� ���������� ������ �����
	// VK_POLYGON_MODE_POINT - ����� ���������� ������ �����
	// VK_POLYGON_MODE_LINE � VK_POLYGON_MODE_POINT ������� ��������� ����������
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

	// ���������� ������� �����, �������� ���������� �����.
	// ����� ������� ������ 1 ������� ��������� ����������
	rasterizer.lineWidth = 1.0f;

	// ���������� ����� ������������ ����� ������������� ��� ������������
	// VK_CULL_MODE_BACK_BIT - �������� ��� �� ����������� ��� ������������, 
	// ������� ��������� � ������ ������� ������
	rasterizer.cullMode  = VK_CULL_MODE_BACK_BIT;

	// ���������� � ����� ������� �� ��������, ����� ����������� ������� ��������, 
	// ������� ������� ������
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;


	rasterizer.depthBiasEnable		   = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp		   = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor    = 0.0f; // Optional
	////////////////////////////////////////////////////////////////////////


	//////////////////////// MULTISAMPLING STAGE ////////////////////////

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType				    = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable   = VK_FALSE; 
	multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading      = 1.0f;     // Optional
	multisampling.pSampleMask		    = nullptr;  // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable      = VK_FALSE; // Optional

	/////////////////////////////////////////////////////////////////////


	//////////////////////// COLOR ATTACHMENT STAGE ////////////////////////

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask		 = VK_COLOR_COMPONENT_R_BIT | 
											   VK_COLOR_COMPONENT_G_BIT | 
											   VK_COLOR_COMPONENT_B_BIT | 
											   VK_COLOR_COMPONENT_A_BIT;

	// ����� ����� ����� ���������� � �������� ����� ���������� ����� ��������� �������� ������
	colorBlendAttachment.blendEnable		 = VK_TRUE;

	// �� ������ ���������� ��� �����, �������� �� ������������
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp		 = VK_BLEND_OP_ADD;

	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp		 = VK_BLEND_OP_ADD;

	// finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
	// finalColor.a = newAlpha.a;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType			    = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

	// ����� �� �� ������������ ���������� �������� ��� ���������� ������
	// colorBlending.logicOpEnable	    == VK_FALSE ������������� ���������
	// colorBlendAttachment.blendEnable
	// ����� ������� �� ����� ��������� ����� ���� ��������� ������ ���������� ��������, ����
	// ������� �������������� �������� ����� ��������
	// ������ ���� ������� � �������� ���� �� ���� ����� ����������. 
	// ����� �� framebuffer �� ������� ������� ������
	colorBlending.logicOpEnable	    = VK_FALSE;
	colorBlending.logicOp		    = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount   = 1;
	colorBlending.pAttachments	    = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	////////////////////////////////////////////////////////////////////////

	// ��������� ��������� pipeline, ����� ��� ������ viewport, ������� ����� � blendConstants 
	// ����� ����������� �� ����, �� ������������ ���� pipeline
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates	   = dynamicStates;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount		  = 1;
	pipelineLayoutInfo.pSetLayouts			  = &descriptorSetLayout;

	pipelineLayoutInfo.pushConstantRangeCount = 0;		 // Optional
	pipelineLayoutInfo.pPushConstantRanges	  = nullptr; // Optional



	if(vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType		= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	
	// ��������� ���������� ������ � �������������� �������� � ��������� �������� ����� ������
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages	= shaderStages;

	// �������� ������ �� �������� ���� ������, ������������� ����
	pipelineInfo.pVertexInputState	 = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState		 = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState   = &multisampling;
	pipelineInfo.pDepthStencilState  = nullptr; // Optional
	pipelineInfo.pColorBlendState    = &colorBlending;
	pipelineInfo.pDynamicState		 = nullptr; // Optional

	pipelineInfo.layout = pipelineLayout;

	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass	= 0; // index of subpass

	// pipelain ����� �������������, ��� ��������� �����. ����� �� ���������� ��� ������ ��� � ����
	// ����� ������� ������ �� pipelain, ������� ����� �������������� ��� ������� ��� ������
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex  = -1;			  // Optional

	// pipelineInfo � pipelineInfo ����� ��������� �� �������
	// ��� ��������� ��������� ����� pipeline �� ���� �����
	if(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}


static std::vector<char> readFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if(!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

///////////////////////////////////////////////////////////

void createFramebuffers(const VkDevice				   &logicalDevice,
						const VkRenderPass			   &renderPass,
						const VkExtent2D			   &swapChainExtent,
						const std::vector<VkImageView> &swapChainImageViews,
						std::vector<VkFramebuffer>	   &swapChainFramebuffers)
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for(size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		// �������� ������. � ����� ������ ������ ����� ������ � ���� ���� ����������� �� 
		// swap chain
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		// ��������� renderPass, � ������� ����� ������� ���� ���������
		framebufferInfo.renderPass		= renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments	= attachments;
		framebufferInfo.width			= swapChainExtent.width;
		framebufferInfo.height			= swapChainExtent.height;
		framebufferInfo.layers			= 1; // ���������� ����� � ����� �����������

		if(vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

///////////////////////////////////////////////////////////

void createCommandPool(const VkPhysicalDevice	&physicalDevice,
					   const VkDevice			&logicalDevice,
					   const VkSurfaceKHR		&surface, 
					   VkCommandPool			&commandPool)
{
	// ���������� ��������� ������� ���������� ����� �� ���������� �� � ���� �� �������� ��������
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType			  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	// ��������� �����:
	// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT - �� ������� ��� ���� ����� ����� ����� ����� �����������
	// (������ ����� �������� ���� ��������� ���������� ��������� ������)
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT - ��������� ��� �������������� ������ 
	// ������ ��� ������. ��� ���� ��� ����� ����� ��������� ������������
	poolInfo.flags = 0; // Optional

	if(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

void createCommandBuffers(const VkDevice				   &logicalDevice,
						  const VkExtent2D				   &swapChainExtent,
						  const std::vector<VkFramebuffer> &swapChainFramebuffers,
						  const VkPipeline				   &graphicsPipeline,
						  const VkRenderPass			   &renderPass,
						  VkBuffer						   &vertexBuffer,
						  VkBuffer						   &indexBuffer,
						  VkCommandPool					   &commandPool,
						  std::vector<VkCommandBuffer>	   &commandBuffers,
						  std::vector<VkDescriptorSet>	   &descriptorSets,
						  VkPipelineLayout				   &pipelineLayout,
						  int								indexBufferSize)
{
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool		 = commandPool;

	// ���� ����� �������� ��������� ��� ���������
	// VK_COMMAND_BUFFER_LEVEL_PRIMARY - ����� �������� ���������.
	// ��� ����� ��������� � ������� ��� ����������, �� ��� ������ �������� �� ������ �������
	// VK_COMMAND_BUFFER_LEVEL_SECONDARY - ����� �������� ���������. �� ����� ����
	// ��������� � ������� ��������, �� ����� ���� ������ �� ��������� �������
	// �� ��������� ������ ����� ������� ����������������� ������� ����� �������� �� 
	// �� �������� ������ � ������ �������
	allocInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for(size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// ����� ��������� �� ��� �� ������� ������������ ���� ��������� �����
		// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT - ����� ����� ����������� ����� �� ����� 
		// ����������
		// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT - ��� ���������� �����, �������
		// ����� �������������� �� ���������� ������ ������� ����������(render pass)
		// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT - ���� ����� ����� ���� 
		// ��������� � ������� � �� �� �����, ���� �� ��� ��������� � ������� � �����������
		beginInfo.flags = 0; // Optional

		// ������������. ����� ������ ��� ��������� ������� � ��������� ��, ����� ��������� �����
		// ������������ � ���������� ������, ��� ������ ���� ���������
		beginInfo.pInheritanceInfo = nullptr; 

		// ����� ������ vkBeginCommandBuffer �������� ���-�� � ����� ������
		if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType	   = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass  = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];

		// ���������� ������� ��� ����������
		// ��� ������ ������������������ ���������� ����� ������� ���������� ���������
		// � �������� ����������� � attahments
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapChainExtent;

		// ���� �������� ��� ����� ������� ����������
		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues	   = &clearColor;

		// ������ �������� ���������� ����� ������� ����� ����������� ������ ������
		// VK_SUBPASS_CONTENTS_INLINE - ��� �������� ��������� � ������� �����
		// � ������� ��������� ������ ������� �� �����
		// VK_SUBPASS_CONTENTS_INLINE - ������� ����� ����������� �� ��������� �������
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// ������ �������� ��������� ��� ���� pipeline ������������ ��� �������
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkBuffer vertexBuffers[] = {vertexBuffer};
		VkDeviceSize offsets[]	 = {0};
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[i], 
								VK_PIPELINE_BIND_POINT_GRAPHICS, 
								pipelineLayout, 
								0, 
								1, 
								&descriptorSets[i], 
								0, 
								nullptr);

		vkCmdDrawIndexed(commandBuffers[i], 
						 static_cast<uint32_t>(indexBufferSize), 
						 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

}


///////////////////////////////////////////////////////////


void createSyncObjects(const VkDevice			  &logicalDevice,
					   int						  MAX_FRAMES_IN_FLIGHT,
					   const std::vector<VkImage> &swapChainImages,
					   std::vector<VkSemaphore>	  &imageAvailableSemaphores,
					   std::vector<VkSemaphore>   &renderFinishedSemaphores,
					   std::vector<VkFence>		  &inFlightFences,
					   std::vector<VkFence>		  &imagesInFlight)
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	// �������� �������� ��������� Vulkan API ��� �������� �������� ����� �� ��������� �������
	// ���������� ����� sType
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// ������� ��� ��� �������� ��� ����� ������ ���� ����� � ���������� ���������
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
		   vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
		   vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

void createDescriptorSetLayout(const VkDevice			&logicalDevice, 
							   VkDescriptorSetLayout	&descriptorSetLayout)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding		 = 0;
	uboLayoutBinding.descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	// ����� ������� ����� �������� ������ ������������?
	// ����� �������� ��������� ������ ��������� |
	// ��� VK_SHADER_STAGE_ALL_GRAPHICS ���� ������ ������������ ���������
	// �� ���� �������
	uboLayoutBinding.stageFlags		 = VK_SHADER_STAGE_VERTEX_BIT;

	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional


	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings	= &uboLayoutBinding;

	if(vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void createDescriptorPool(const VkDevice			 &logicalDevice,
						  const std::vector<VkImage> &swapChainImages,
						  VkDescriptorPool			 &descriptorPool)
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type			 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount  = 1;
	poolInfo.pPoolSizes		= &poolSize;
	poolInfo.maxSets		= static_cast<uint32_t>(swapChainImages.size());


	if(vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool!");
}

void createDescriptorSets(const VkDevice			   &logicalDevice,
						  const std::vector<VkImage>   &swapChainImages,
						  VkDescriptorPool			   &descriptorPool,
						  VkDescriptorSetLayout		   &descriptorSetLayout,
						  std::vector<VkDescriptorSet> &descriptorSets,
						  std::vector<VkBuffer>		   &uniformBuffers)
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
	
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType				 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool	 = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts		 = layouts.data();

	descriptorSets.resize(swapChainImages.size());
	if(vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate descriptor sets!");

	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		// ���� �� �������� ������ ��� �������������� ����� �������, 
		// �� �� ����� ������������ VK_WHOLE_SIZE ��� range
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet			= descriptorSets[i];
		descriptorWrite.dstBinding		= 0;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo		 = &bufferInfo;
		descriptorWrite.pImageInfo		 = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);
	}
}
