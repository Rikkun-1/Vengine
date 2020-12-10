#include "renderPass.h"

#include "image.h"

static void fillColorAttachmentDescription(VkAttachmentDescription &colorAttachment)
{
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
}

static void setupAttachmentRef(uint32_t              attachmentIndex,
                               VkImageLayout         layout, 
                               VkAttachmentReference &attachmentRef)
{
    attachmentRef.attachment = attachmentIndex; // ������ attachment �� ������� �� ���������
    // ��� ����� ����� ����� �������������� ���� attachment ��������������� ������
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL - ������������� ��� ��������� � ����
    attachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

static void setupSubpassDescription(const VkAttachmentReference &colorAttachmentRef,
                                    const VkAttachmentReference &depthAttachmentRef,
                                    VkSubpassDescription        &subpass)
{
    // ��� �������� ������ ������
    // VK_PIPELINE_BIND_POINT_GRAPHICS - ������ ������ �������� � ��������
    // VK_PIPELINE_BIND_POINT_COMPUTE - ������ ������ �������� � ������������
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // ������ attachment'� ��� � ���� ���� �� ������� �� ������ �� ����� ������ � ��������
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
    
    // � ���� ������ ����� ���� �������� ��������� ����������:
    // pInputAttachments: ��� ���������� ����� �������������� ��� ���� ��� ��������
    // pResolveAttachments : ��� ���������� ������������ ��� ���������������
    // pDepthStencilAttachment : ��� ������������ ����� ��� ����� ������ � ���������
    // pPreserveAttachments : ���� ���� �� ������������ �� ����� �������� �������, 
    // �� ��� ������ ������ ���� ������������� � �������������
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