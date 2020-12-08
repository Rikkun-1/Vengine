#include "commandBuffer.h"

void createCommandBuffers(VkDevice                     logicalDevice,
                          VkExtent2D                   swapChainExtent,
                          std::vector<VkFramebuffer>   &swapChainFramebuffers,
                          VkPipeline                   graphicsPipeline,
                          VkRenderPass                 renderPass,
                          VkBuffer                     vertexBuffer,
                          VkBuffer                     indexBuffer,
                          VkCommandPool                commandPool,
                          std::vector<VkCommandBuffer> &commandBuffers,
                          std::vector<VkDescriptorSet> &descriptorSets,
                          VkPipelineLayout             pipelineLayout,
                          int                          indexBufferSize)
{
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;

    // ���� ����� �������� ��������� ��� ���������
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY - ����� �������� ���������.
    // ��� ����� ��������� � ������� ��� ����������, �� ��� ������ �������� �� ������ �������
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY - ����� �������� ���������. �� ����� ����
    // ��������� � ������� ��������, �� ����� ���� ������ �� ��������� �������
    // �� ��������� ������ ����� ������� ����������������� ������� ����� �������� �� 
    // �� �������� ������ � ������ �������
    allocInfo.level               = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

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
            throw std::runtime_error("failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass  = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];

        // ���������� ������� ��� ����������
        // ��� ������ ������������������ ���������� ����� ������� ���������� ���������
        // � �������� ����������� � attahments
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

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
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool         = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer,
                           VkDevice        logicalDevice,
                           VkCommandPool   commandPool,
                           VkQueue         graphicsQueue)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}
