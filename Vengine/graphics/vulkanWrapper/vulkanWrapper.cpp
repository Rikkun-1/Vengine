#include "vulkanWrapper.h"

void createSwapChain(const LogicalDevice &device,
                     VkSurfaceKHR        surface,
                     VkExtent2D          &requiredExtent,
                     SwapChain           &swapChain)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport  (device.physicalDevice, surface);
    VkSurfaceFormatKHR      surfaceFormat    = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR        presentMode      = chooseSwapPresentMode  (swapChainSupport.presentModes);
    
   
    VkExtent2D actualExtent = chooseSwapExtent(requiredExtent, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    //maxImageCount ������ ���� �������� ��� ����� ��� ����������� �� ���������� �����������
    if(swapChainSupport.capabilities.maxImageCount > 0  &&
       imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = actualExtent;
    createInfo.imageArrayLayers = 1; // ���������� ����� �� ������� ����� �������������

    // ������ ������� ������ ��������������� ��������������� ��� ��������� ����� � ���
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices    = findQueueFamilies(device.physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if(indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices   = nullptr; // Optional
    }

    createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device.handle, &createInfo, nullptr, &swapChain.handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");

    vkGetSwapchainImagesKHR(device.handle, swapChain.handle, &imageCount, nullptr);

    swapChain.images.resize(imageCount);
    vkGetSwapchainImagesKHR(device.handle, swapChain.handle, &imageCount, swapChain.images.data());

    createImageViews(device, swapChain);

    swapChain.imageFormat = surfaceFormat.format;
    swapChain.extent      = actualExtent;
}


VkRenderPass createRenderPass(const LogicalDevice &device,
                              VkFormat            swapChainImageFormat)
{
    VkAttachmentDescription colorAttachment;
    fillColorAttachmentDescription(colorAttachment);
    colorAttachment.format = swapChainImageFormat;


    VkAttachmentDescription depthAttachment;
    fillDepthAttachmentDescription(depthAttachment);
    depthAttachment.format = findDepthFormat(device.physicalDevice);


    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // ������ attachment �� ������� �� ���������
    // ��� ����� ����� ����� �������������� ���� attachment ��������������� ������
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL - ������������� ��� ��������� � ����
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


    // �������� �������� ����������
    VkSubpassDescription subpass{};
    // ��� �������� ������ ������
    // VK_PIPELINE_BIND_POINT_GRAPHICS - ������ ������ �������� � ��������
    // VK_PIPELINE_BIND_POINT_COMPUTE - ������ ������ �������� � ������������
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // ������ attachment'� ��� � ���� ���� �� ������� �� ������ �� ����� ������ � ��������
    // layout(location = 0) out vec4 outColor
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;


    VkSubpassDependency dependency{};
    // ������ ���������� ��� �������� �� ����������� �����������
    // VK_SUBPASS_EXTERNAL ��������� ������� ���������� ������� ���������� �� ��� ����� 
    // ������ render pass � ����������� �� ���� ���������� �� � srcSubpass ��� dstSubpass
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

    // ������ ��������� ����������. ��������� � ��� ���� ������������ � ��� ������� ����� 0
    dependency.dstSubpass = 0;
    //dstSubpass ������ ������ ���� ������ ��� srcSubpass �� ��������� ������������ � ����� ������������

    // ����� �� ��������� �� ���� �� ���� ������ ��� ������ � �� ����� ������ �� ����� ����
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT          | 
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


    // � ���� ������ ����� ���� �������� ��������� ����������:
    // pInputAttachments: ��� ���������� ����� �������������� ��� ���� ��� ��������
    // pResolveAttachments : ��� ���������� ������������ ��� ���������������
    // pDepthStencilAttachment : ��� ������������ ����� ��� ����� ������ � ���������
    // pPreserveAttachments : ���� ���� �� ������������ �� ����� �������� �������, 
    // �� ��� ������ ������ ���� ������������� � �������������
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;


    VkRenderPass renderPass;
    if(vkCreateRenderPass(device.handle, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("failed to create render pass!");

    return renderPass;
}


///////////////////////////////////////////////////////////

void createDescriptorSetLayout(VkDevice              logicalDevice, 
                               VkDescriptorSetLayout &descriptorSetLayout)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding         = 0;
    uboLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    // ����� ������� ����� �������� ������ ������������?
    // ����� �������� ��������� ������ ��������� |
    // ��� VK_SHADER_STAGE_ALL_GRAPHICS ���� ������ ������������ ���������
    // �� ���� �������
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding            = 1;
    samplerLayoutBinding.descriptorCount    = 1;
    samplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

    if(vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");
}

void createGraphicsPipeline(VkDevice              logicalDevice,
                            VkExtent2D            swapChainExtent,
                            VkRenderPass          renderPass,
                            VkPipelineLayout      &pipelineLayout,
                            VkPipeline            &graphicsPipeline,
                            VkDescriptorSetLayout &descriptorSetLayout)
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

    auto bindingDescription    = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();
    
    ////////////////////////////////////////////////////////////////////


    //////////////////////// INPUT ASSEMBLY STAGE ////////////////////////

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
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
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;
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


    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp          = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor    = 0.0f; // Optional
    ////////////////////////////////////////////////////////////////////////


    //////////////////////// MULTISAMPLING STAGE ////////////////////////

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE; 
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;     // Optional
    multisampling.pSampleMask           = nullptr;  // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable      = VK_FALSE; // Optional

    /////////////////////////////////////////////////////////////////////

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable       = VK_TRUE;
    // ������ �� ��������, ������� ������ ���� ������� ���������� ��� ����� �������
    depthStencil.depthWriteEnable      = VK_TRUE;
    depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;

    // ��������� ���������� ���� � ����� �������� ������� ��������� ������ �������� ����� ������ ����
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds        = 0.0f; // Optional
    depthStencil.maxDepthBounds        = 1.0f; // Optional

    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front             = {}; // Optional
    depthStencil.back              = {}; // Optional

    //////////////////////// COLOR ATTACHMENT STAGE ////////////////////////

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                          VK_COLOR_COMPONENT_G_BIT | 
                                          VK_COLOR_COMPONENT_B_BIT | 
                                          VK_COLOR_COMPONENT_A_BIT;

    // ����� ����� ����� ���������� � �������� ����� ���������� ����� ��������� �������� ������
    colorBlendAttachment.blendEnable = VK_TRUE;

    // �� ������ ���������� ��� �����, �������� �� ������������
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;

    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    // finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
    // finalColor.a = newAlpha.a;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    // ����� �� �� ������������ ���������� �������� ��� ���������� ������
    // colorBlending.logicOpEnable == VK_FALSE ������������� ���������
    // colorBlendAttachment.blendEnable
    // ����� ������� �� ����� ��������� ����� ���� ��������� ������ ���������� ��������, ����
    // ������� �������������� �������� ����� ��������
    // ������ ���� ������� � �������� ���� �� ���� ����� ����������. 
    // ����� �� framebuffer �� ������� ������� ������
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
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
    dynamicState.pDynamicStates    = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;

    pipelineLayoutInfo.pushConstantRangeCount = 0;       // Optional
    pipelineLayoutInfo.pPushConstantRanges    = nullptr; // Optional



    if(vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    // ��������� ���������� ������ � �������������� �������� � ��������� �������� ����� ������
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages    = shaderStages;

    // �������� ������ �� �������� ���� ������, ������������� ����
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = nullptr; // Optional

    pipelineInfo.layout = pipelineLayout;

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass    = 0; // index of subpass

    // pipelain ����� �������������, ��� ��������� �����. ����� �� ���������� ��� ������ ��� � ����
    // ����� ������� ������ �� pipelain, ������� ����� �������������� ��� ������� ��� ������
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex  = -1;             // Optional

    // pipelineInfo � pipelineInfo ����� ��������� �� �������
    // ��� ��������� ��������� ����� pipeline �� ���� �����
    if(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");

    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

///////////////////////////////////////////////////////////

void createFramebuffers(VkDevice                       logicalDevice,
                        VkRenderPass                   renderPass,
                        VkExtent2D                     swapChainExtent,
                        const std::vector<VkImageView> &swapChainImageViews,
                        std::vector<VkFramebuffer>     &swapChainFramebuffers,
                        VkImageView                    depthImageView)
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for(size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        // �������� ������. � ����� ������ ������ ����� ������ � ���� ���� ����������� �� 
        // swap chain
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments    = attachments.data();
        framebufferInfo.width           = swapChainExtent.width;
        framebufferInfo.height          = swapChainExtent.height;
        framebufferInfo.layers          = 1;

        if(vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}

///////////////////////////////////////////////////////////

void createCommandPool(VkPhysicalDevice physicalDevice,
                       VkDevice         logicalDevice,
                       VkSurfaceKHR     surface, 
                       VkCommandPool    &commandPool)
{
    // ���������� ��������� ������� ���������� ����� �� ���������� �� � ���� �� �������� ��������
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    // ��������� �����:
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT - �� ������� ��� ���� ����� ����� ����� ����� �����������
    // (������ ����� �������� ���� ��������� ���������� ��������� ������)
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT - ��������� ��� �������������� ������ 
    // ������ ��� ������. ��� ���� ��� ����� ����� ��������� ������������
    poolInfo.flags = 0; // Optional

    if(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
}

///////////////////////////////////////////////////////////


void createSyncObjects(VkDevice                   logicalDevice,
                       int                        MAX_FRAMES_IN_FLIGHT,
                       const std::vector<VkImage> &swapChainImages,
                       std::vector<VkSemaphore>   &imageAvailableSemaphores,
                       std::vector<VkSemaphore>   &renderFinishedSemaphores,
                       std::vector<VkFence>       &inFlightFences,
                       std::vector<VkFence>       &imagesInFlight)
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


void createDescriptorPool(VkDevice                   logicalDevice,
                          const std::vector<VkImage> swapChainImages,
                          VkDescriptorPool           &descriptorPool)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());


    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = static_cast<uint32_t>(swapChainImages.size());


    if(vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool!");


}

void createDescriptorSets(VkDevice                     logicalDevice,
                          const std::vector<VkImage>   swapChainImages,
                          VkDescriptorPool             descriptorPool,
                          VkDescriptorSetLayout        descriptorSetLayout,
                          std::vector<VkDescriptorSet> &descriptorSets,
                          std::vector<VkBuffer>        &uniformBuffers,
                          VkImageView                  textureImageView,
                          VkSampler                    textureSampler)
{
    std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts        = layouts.data();

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

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = textureImageView;
        imageInfo.sampler     = textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet          = descriptorSets[i];
        descriptorWrites[0].dstBinding      = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet          = descriptorSets[i];
        descriptorWrites[1].dstBinding      = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(logicalDevice, 
                               static_cast<uint32_t>(descriptorWrites.size()), 
                               descriptorWrites.data(), 
                               0, 
                               nullptr);
    }
}

