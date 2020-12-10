#include "commandBuffer.h"

#include <array>
#include <stdexcept>

#include "buffer.h"

static void allocateCommandBuffers(VkDevice         logicalDevice,
                                   VkCommandPool    commandPool,
                                   uint32_t         amount,
                                   VkCommandBuffer  *commandBuffers)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;

    // ���� ����� �������� ��������� ��� ���������
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY - ����� �������� ���������.
    // ��� ����� ��������� � ������� ��� ����������, �� ��� ������ �������� �� ������ �������
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY - ����� �������� ���������. �� ����� ����
    // ��������� � ������� ��������, �� ����� ���� ������ �� ��������� �������
    // �� ��������� ������ ����� ������� ����������������� ������� ����� �������� �� 
    // �� �������� ������ � ������ �������
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = amount;

    if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");
}

static void beginCommandBuffer(VkCommandBuffer            commandBuffer,
                               VkCommandBufferUsageFlags  flags = 0) // optional
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
    beginInfo.flags = flags; // Optional

    // ������������. ����� ������ ��� ��������� ������� � ��������� ��, ����� ��������� �����
    // ������������ � ���������� ������, ��� ������ ���� ���������
    beginInfo.pInheritanceInfo = nullptr;

    // ����� ������ vkBeginCommandBuffer �������� ���-�� � ����� ������
    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");
}

static void beginRenderPass(VkRenderPass        renderPass,
                            VkCommandBuffer     commandBuffer,
                            VkFramebuffer       frameBuffer,
                            const VkExtent2D    &extent)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass  = renderPass;
    renderPassInfo.framebuffer = frameBuffer;

    // ���������� ������� ��� ����������
    // ��� ������ ������������������ ���������� ����� ������� ���������� ���������
    // � �������� ����������� � attahments
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    // ����� �������� ��� ����� ������� ����������
    // ����� ����� ������� �������� � clearValues �������������� ������� ������������
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color        = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    // ������ �������� ���������� ����� ������� ����� ����������� ������ ������
    // VK_SUBPASS_CONTENTS_INLINE - ��� �������� ��������� � ������� �����
    // � ������� ��������� ������ ������� �� �����
    // VK_SUBPASS_CONTENTS_INLINE - ������� ����� ����������� �� ��������� �������
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void createCommandBuffers(VkDevice                     logicalDevice,
                          SwapChain                    &swapChain,
                          VkRenderPass                 renderPass,
                          VkPipeline                   graphicsPipeline,
                          VkPipelineLayout             pipelineLayout,
                          VkBuffer                     vertexBuffer,
                          VkBuffer                     indexBuffer,
                          std::vector<VkDescriptorSet> &descriptorSets,
                          int                          indexBufferSize,
                          VkCommandPool                commandPool,
                          std::vector<VkCommandBuffer> &commandBuffers)
{
    commandBuffers.resize(swapChain.frameBuffers.size());
    allocateCommandBuffers(logicalDevice, 
                           commandPool, 
                           commandBuffers.size(),
                           commandBuffers.data());


    for(size_t i = 0; i < commandBuffers.size(); i++)
    {
        beginCommandBuffer(commandBuffers[i]);

        beginRenderPass(renderPass, 
                        commandBuffers[i], 
                        swapChain.frameBuffers[i], 
                        swapChain.extent);

        // ������ �������� ��������� ��� ���� pipeline ������������ ��� �������
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
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
            throw std::runtime_error("failed to record command buffer!");
    }
}


VkCommandBuffer beginSingleTimeCommands(VkDevice      logicalDevice,
                                        VkCommandPool commandPool)
{
    VkCommandBuffer commandBuffer;
    allocateCommandBuffers(logicalDevice, 
                           commandPool, 
                           1,
                           &commandBuffer);

    beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    return commandBuffer;
}


void endSingleTimeCommands(const LogicalDevice  &device,
                           VkCommandBuffer      commandBuffer,
                           VkCommandPool        commandPool)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.graphicsQueue);

    vkFreeCommandBuffers(device.handle, commandPool, 1, &commandBuffer);
}


VkCommandPool createCommandPool(const LogicalDevice  &device,
                                VkSurfaceKHR         surface)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // ���������� ��������� ������� ���������� ����� �� ���������� �� � ���� �� �������� ��������
    poolInfo.queueFamilyIndex = device.familyIndices.graphicsFamily.value();

    // ��������� �����:
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT - �� ������� ��� ���� ����� ����� ����� ����� �����������
    // (������ ����� �������� ���� ��������� ���������� ��������� ������)
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT - ��������� ��� �������������� ������ 
    // ������ ��� ������. ��� ���� ��� ����� ����� ��������� ������������
    poolInfo.flags = 0; // Optional

    VkCommandPool commandPool;
    if(vkCreateCommandPool(device.handle, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");

    return commandPool;
}
