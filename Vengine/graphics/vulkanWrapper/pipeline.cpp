#include "pipeline.h"

struct PipelineFixedFunctions
{
    VkPipelineVertexInputStateCreateInfo         vertexInput;
    VkPipelineInputAssemblyStateCreateInfo       inputAssembly;
    VkViewport                                   viewport;       
    VkRect2D                                     scissor;
    VkPipelineViewportStateCreateInfo            viewportState;
    VkPipelineRasterizationStateCreateInfo       rasterizer;
    VkPipelineMultisampleStateCreateInfo         multisampling;
    VkPipelineDepthStencilStateCreateInfo        depthStencil;
    VkPipelineColorBlendAttachmentState          colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo          colorBlending;
    VkPipelineDynamicStateCreateInfo             dynamicState;



};

static void setupShaderStageInfo(const ShaderModule               &shader,
                                 VkPipelineShaderStageCreateInfo  &shaderStage)
{
    shaderStage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage  = shader.stage;
    shaderStage.module = shader.handle;
    shaderStage.pName  = shader.entry.c_str(); // ����� �����

    // ������������ ��������, ������� ��������� ���������� ��������� �������� � ������
    // �������� �� ��� �������� ������ ����� ������ ���� ���������
    shaderStage.pSpecializationInfo = nullptr;
}

static void setupMultipleShaderStages(const ShaderModule          &vertexShader,
                                      const ShaderModule          &fragmentShader,
                                      std::vector<VkPipelineShaderStageCreateInfo> &shaderStages) 
{
    VkPipelineShaderStageCreateInfo  vertexStageInfo{};
    VkPipelineShaderStageCreateInfo  fragmentStageInfo{};
   
    setupShaderStageInfo(vertexShader,   vertexStageInfo);
    setupShaderStageInfo(fragmentShader, fragmentStageInfo);
    
    shaderStages.push_back(vertexStageInfo);
    shaderStages.push_back(fragmentStageInfo);
}

static void setupAssemblyStateInfo(VkPipelineInputAssemblyStateCreateInfo &inputAssembly)
{
    inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
}

static void setupVertexInputDescriptions(VkPipelineVertexInputStateCreateInfo &vertexInput)
{
    static auto bindingDescription    = Vertex::getBindingDescription();
    static auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount   = 1;
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInput.pVertexBindingDescriptions      = &bindingDescription;
    vertexInput.pVertexAttributeDescriptions    = attributeDescriptions.data();
}

static void setupViewPortAndScissor(const VkExtent2D  &swapChainExtent,
                                    VkViewport        &viewport, 
                                    VkRect2D          &scissor)
    {
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width  = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
    }

static void setupViewPortStateInfo(VkViewport                         &viewport, 
                                   VkRect2D                           &scissor,
                                   VkPipelineViewportStateCreateInfo  &viewportState)
{
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;
    //������� ��������� viewport ��� scissors ������� ���������
}

static void setupRasterizerStateInfo(VkPipelineRasterizationStateCreateInfo  &rasterizer)
{
    rasterizer.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
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
}

static void setupMultisamplingStateInfo(VkPipelineMultisampleStateCreateInfo &multisampling)
{
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE; 
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;     // Optional
    multisampling.pSampleMask           = nullptr;  // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable      = VK_FALSE; // Optional
}

static void setupDepthStencilStateInfo(VkPipelineDepthStencilStateCreateInfo  &depthStencil)
{
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable       = VK_TRUE;
    // ������ �� ��������, ������� ������ ���� ������� ���������� ��� ����� �������
    depthStencil.depthWriteEnable      = VK_TRUE;
    depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;

    // ��������� ���������� ���� � ����� �������� ������� ��������� ������ �������� ����� ������ ����
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    //depthStencil.minDepthBounds        = 0.0f; // Optional
    //depthStencil.maxDepthBounds        = 1.0f; // Optional

    depthStencil.stencilTestEnable = VK_FALSE;
    //depthStencil.front             = {}; // Optional
    //depthStencil.back              = {}; // Optional
}
        
static void setupColorAttachmentState(VkPipelineColorBlendAttachmentState    &colorBlendAttachment)
{
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                          VK_COLOR_COMPONENT_G_BIT | 
                                          VK_COLOR_COMPONENT_B_BIT | 
                                          VK_COLOR_COMPONENT_A_BIT;

    // ����� ����� ����� ���������� � �������� ����� ���������� ����� ��������� �������� ������
    colorBlendAttachment.blendEnable = false;

    // �� ������ ���������� ��� �����, �������� �� ������������
    //colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    //colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    //colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;

    //colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    //colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    //colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    // finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
    // finalColor.a = newAlpha.a;
}

static void setupColorBlendStateInfo(VkPipelineColorBlendStateCreateInfo        &colorBlending,
                                     const VkPipelineColorBlendAttachmentState  &colorBlendAttachment) 
{
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
}

static void setupDynamicStates(VkPipelineDynamicStateCreateInfo  &dynamicState) 
{   
    // ��������� ��������� pipeline, ����� ��� ������ viewport, ������� ����� � blendConstants 
    // ����� ����������� �� ����, �� ������������ ���� pipeline
    static std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();
}

static void setupFixedFunctions(const VkExtent2D            &swapChainExtent,
                                PipelineFixedFunctions      &fixedFunctions)
{
    setupVertexInputDescriptions(fixedFunctions.vertexInput);
    setupAssemblyStateInfo      (fixedFunctions.inputAssembly);
    setupViewPortAndScissor     (swapChainExtent,
                                 fixedFunctions.viewport, 
                                 fixedFunctions.scissor);
    setupViewPortStateInfo      (fixedFunctions.viewport, 
                                 fixedFunctions.scissor,
                                 fixedFunctions.viewportState);
    setupRasterizerStateInfo    (fixedFunctions.rasterizer);
    setupMultisamplingStateInfo (fixedFunctions.multisampling);
    setupDepthStencilStateInfo  (fixedFunctions.depthStencil);
    setupColorAttachmentState   (fixedFunctions.colorBlendAttachment);
    setupColorBlendStateInfo    (fixedFunctions.colorBlending, 
                                 fixedFunctions.colorBlendAttachment);

    //setupDynamicStates          (fixedFunctions.dynamicState);
}

VkPipeline createGraphicsPipeline(const LogicalDevice         &device,
                                  VkExtent2D                  &swapChainExtent,
                                  VkRenderPass                renderPass,
                                  const ShaderModule          &vertexShader,
                                  const ShaderModule          &fragmentShader,
                                  const VkDescriptorSetLayout &descriptorSetLayout,
                                  const VkPipelineLayout      &pipelineLayout)
{
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    
    setupShaderStageInfo(vertexShader, vertShaderStageInfo);
    setupShaderStageInfo(fragmentShader, fragShaderStageInfo);

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    PipelineFixedFunctions fixedFunctions{};
    setupFixedFunctions(swapChainExtent, fixedFunctions);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
    pipelineInfo.pVertexInputState   = &fixedFunctions.vertexInput;
    pipelineInfo.pInputAssemblyState = &fixedFunctions.inputAssembly;
    pipelineInfo.pViewportState      = &fixedFunctions.viewportState;
    pipelineInfo.pRasterizationState = &fixedFunctions.rasterizer;
    pipelineInfo.pMultisampleState   = &fixedFunctions.multisampling;
    pipelineInfo.pDepthStencilState  = &fixedFunctions.depthStencil;
    pipelineInfo.pColorBlendState    = &fixedFunctions.colorBlending;
    pipelineInfo.layout              = pipelineLayout;
    pipelineInfo.renderPass          = renderPass;
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(device.handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return graphicsPipeline;
}

/*

*/
/*
*   std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
    setupMultipleShaderStages(vertexShader, fragmentShader, shaderStages);
  
    PipelineFixedFunctions fixedFunctions{};
    setupFixedFunctions(swapChainExtent,
                        fixedFunctions);


    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    // ��������� ���������� ������ � �������������� �������� � ��������� �������� ����� ������
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages    = shaderStages.data();

    // �������� ������ �� �������� ���� ������
    pipelineInfo.pVertexInputState   = &fixedFunctions.vertexInput;
    pipelineInfo.pInputAssemblyState = &fixedFunctions.inputAssembly;
    pipelineInfo.pViewportState      = &fixedFunctions.viewportState;
    pipelineInfo.pRasterizationState = &fixedFunctions.rasterizer;
    pipelineInfo.pMultisampleState   = &fixedFunctions.multisampling;
    pipelineInfo.pDepthStencilState  = &fixedFunctions.depthStencil;
    pipelineInfo.pColorBlendState    = &fixedFunctions.colorBlending;
    //pipelineInfo.pDynamicState       = &fixedFunctions.dynamicState; 

    pipelineInfo.layout     = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass    = 0; // index of subpass

    // pipelain ����� �������������, ��� ��������� �����. ����� �� ��������� ��� ������ ��� � ����
    // ����� ������� ������ �� pipelain, ������� ����� �������������� ��� ������� ��� ������
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex  = -1;             // Optional

    VkPipeline graphicsPipeline;
    // pipelineInfo � pipelineInfo ����� ��������� �� �������
    // ��� ��������� ��������� ����� pipeline �� ���� �����
    if(vkCreateGraphicsPipelines(device.handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to setup graphics pipeline!");
*/