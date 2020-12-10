#include "renderPass.h"

#include "image.h"

static void fillColorAttachmentDescription(VkAttachmentDescription &colorAttachment)
{
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // что делать с данными во до рендеринга
    // VK_ATTACHMENT_LOAD_OP_CLEAR - зачистить буфер до рендеринга. Изображение начинается с черного полотна
    // VK_ATTACHMENT_LOAD_OP_LOAD  - оставить как есть
    // VK_ATTACHMENT_LOAD_OP_DONT_CARE - нам не важно
    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;

    // что делать после рендеринга
    // VK_ATTACHMENT_STORE_OP_STORE - сохранить буфер для последующего использования. Например для показа
    // VK_ATTACHMENT_STORE_OP_DONT_CARE - нас не заботит что произойдет с результатом после рендеринга
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // тоже самое для буфера - трафарета. Мы его пока не используем. Нас не заботит его состояние
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // данный параметр позволяет представлять изображения в буфере наиболее эфективным способом.
    // Т.е. мы говорим что мы хотим делать с изображением и видеокарта будет хранить его в том виде,
    // который будет наиболее оптимальным для данной операции
    // initialLayout - для каких целей это изображение использовалось ранее
    // finalLayout - для каких целей это изображение будет использоваться после всех операций
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR - это изображение будет ресурсом при отправке в swap chain
    // мы хотим чтобы после обработки это изображение сразу же было готово к отправке в swap chain
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL - мы намерены просто рисовать сюда
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL - это изображение пункт назначения для копирования
    // очень важно чтобы изображения находились в подходящем состоянии для нужных операций
    // VK_IMAGE_LAYOUT_UNDEFINED - мы не знаем в каком состоянии было изначальное изображение
    // это чревато тем что изображение может быть в неправильном состоянии для данной задачи
    // нас это не заботит так как мы все равно зачистим изображение 
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

static void fillDepthAttachmentDescription(VkAttachmentDescription &depthAttachment)
{
    depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

static void fillSubpassDependencies(VkSubpassDependency &dependency)
{
    // индекс подпрохода для которого мы настраиваем зависимости
    // VK_SUBPASS_EXTERNAL описывает неявные подпроходы которые происходят до или после 
    // нашего render pass в зависимости от того расположен он в srcSubpass или dstSubpass
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

    // индекс выходного подпрохода. Подпроход у нас один единственный и его индекст равен 0
    dependency.dstSubpass = 0;
    //dstSubpass всегда должен быть больше чем srcSubpass во избежание зацикливания в графе зависимостей

    // здесь мы описываем то чего мы ждем прежде чем начать и на какой стадии мы этого ждем
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT          | 
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
}

static void setupAttachmentRef(uint32_t              attachmentIndex,
                               VkImageLayout         layout, 
                               VkAttachmentReference &attachmentRef)
{
    attachmentRef.attachment = attachmentIndex; // индекс attachment на которое мы ссылаемся
    // для каких целей будет использоваться этот attachment непосредственно сейчас
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL - оптимизорован для рисования в него
    attachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

static void setupSubpassDescription(const VkAttachmentReference &colorAttachmentRef,
                                    const VkAttachmentReference &depthAttachmentRef,
                                    VkSubpassDescription        &subpass)
{
    // чем является данный проход
    // VK_PIPELINE_BIND_POINT_GRAPHICS - данный проход работает с графикой
    // VK_PIPELINE_BIND_POINT_COMPUTE - данный проход работает с вычислениями
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // индекс attachment'а это и есть слой на который мы рисуем во время работы с шейдером
    // layout(location = 0) out vec4 outColor
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
}


VkRenderPass createRenderPass(const LogicalDevice &device,
                              VkFormat            swapChainImageFormat)
{
    VkAttachmentDescription colorAttachment;
    VkAttachmentDescription depthAttachment;

    fillColorAttachmentDescription(colorAttachment);
    fillDepthAttachmentDescription(depthAttachment);

    colorAttachment.format = swapChainImageFormat;
    depthAttachment.format = findDepthFormat(device.physicalDevice);
    
    // В один проход может быть помещено несколько приложений:
    // pInputAttachments: это приложение будет использоваться как ввод для шейдеров
    // pResolveAttachments : это приложение используется для мультисемплинга
    // pDepthStencilAttachment : это прикрепление нужно для карты глубин и трафарета
    // pPreserveAttachments : этот слой не используется во время текущего прохода, 
    // но его данные должны быть приготовленны к использованию
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};


    VkAttachmentReference colorAttachmentRef{};
    VkAttachmentReference depthAttachmentRef{};

    setupAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         colorAttachmentRef);
    setupAttachmentRef(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depthAttachmentRef);

    
    VkSubpassDescription subpass{};
    setupSubpassDescription(colorAttachmentRef, depthAttachmentRef, subpass);
    

    VkSubpassDependency dependency{};
    fillSubpassDependencies(dependency);


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