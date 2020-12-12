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
    shaderStage.pName  = shader.entry.c_str(); // точка входа

    // опциональный параметр, который позволяет передавать некоторые значения в шейдер
    // опираясь на эти значения шейдер может менять свое поведение
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
    //наличие несколких viewport или scissors требует раширение
}

static void setupRasterizerStateInfo(VkPipelineRasterizationStateCreateInfo  &rasterizer)
{
    rasterizer.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // VK_TRUE требует расширения

    // отключение этапа растеризации. Если VK_TRUE, то 
    // во framebuffer вообще ничего не будет записано
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    // определяет каким образом будет обрабатываться каждый полигон
    // в нашем случае мы заполняем его целиком
    // VK_POLYGON_MODE_LINE  - будут рисоваться только линии
    // VK_POLYGON_MODE_POINT - будут рисоваться только точки
    // VK_POLYGON_MODE_LINE и VK_POLYGON_MODE_POINT требует включение расширения
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    // определяет толщину линий, которыми связваются точки.
    // любая толщина больше 1 требует включение расширения
    rasterizer.lineWidth = 1.0f;

    // определяет какие треугольники будут отбрасываться при растеризации
    // VK_CULL_MODE_BACK_BIT - означает что мы отбрасываем все треугольники, 
    // которые находятся с задней стороны модели
    rasterizer.cullMode  = VK_CULL_MODE_BACK_BIT;

    // определяет в какую сторону мы движемся, когда индексируем вершины полигона, 
    // который смотрит вперед
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
    // должен ли фрагмент, который прошел тест глубины записаться как самый ближний
    depthStencil.depthWriteEnable      = VK_TRUE;
    depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;

    // позволяет определить окно в рамки которого глубина фрагмента должна попадать чтобы пройти тест
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

    // здесь очень много параметров с которыми можно поиграться чтобы настроить смешение цветов
    colorBlendAttachment.blendEnable = false;

    // мы просто складываем два цвета, опираясь на прозрачность
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

    // будем ли мы использовать логические операции при смешивании цветов
    // colorBlending.logicOpEnable == VK_FALSE автоматически отключает
    // colorBlendAttachment.blendEnable
    // Таким образом мы можем смешивать цвета либо используя только логические операции, либо
    // обычные математические операции вроде сложения
    // должен быть включен и настроен хотя бы один метод смешивания. 
    // Иначе во framebuffer не попадут никакие данные
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
    // Некоторые настройки pipeline, такие как размер viewport, толщина линий и blendConstants 
    // можно настраивать на ходу, не пересоздавая весь pipeline
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
    
    // описываем количество стадий с использованием шейдеров и прилагаем описания самих стадий
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages    = shaderStages.data();

    // передаем ссылки на описания всех стадий
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

    // pipelain можно пересоздавать, при изменении задач. Чтобы не создавать его каждый раз с нуля
    // можно указать ссылку на pipelain, который будет использоваться как базовый для нового
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex  = -1;             // Optional

    VkPipeline graphicsPipeline;
    // pipelineInfo и pipelineInfo могут указывать на массивы
    // это позволяет создавать много pipeline за один вызов
    if(vkCreateGraphicsPipelines(device.handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to setup graphics pipeline!");
*/