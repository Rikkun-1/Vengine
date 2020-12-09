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

    // Этот буфер является первичным или вторичным
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY - буфер является первичным.
    // его можно отправить в очередь для исполнения, но его нельзя вызывать из других буферов
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY - буфер является вторичным. Не может быть
    // отправлен в очередь напрямую, но может быть вызван из первичных буферов
    // во вторичном буфере можно описать частоиспользуемые команды чтобы вызывать их 
    // из главного буфера в нужные моменты
    allocInfo.level               = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

    for(size_t i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        // флаги описывают то как мы намеены использовать этот командный буфер
        // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT - буфер будет перезаписан сразу же после 
        // исполнения
        // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT - это вторичныйы буфер, который
        // будет использоваться на протяжении одного прохода рендеринга(render pass)
        // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT - этот буфер может быть 
        // отправлен в очередь в то же время, пока он уже находится в очереди и исполняется
        beginInfo.flags = 0; // Optional

        // опциональный. Нужен только для вторичных буферов и описывает то, какие параметры нужно
        // унаследовать у первичного буфера, что вызвал этот вторичный
        beginInfo.pInheritanceInfo = nullptr;

        // После вызова vkBeginCommandBuffer дописать что-то в буфер нельзя
        if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass  = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];

        // определяем область для рендеринга
        // для лучшей производительности желательно чтобы область рендеринга совпадала
        // с размером изображений в attahments
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        // цвета зачистки для этого прохода рендеринга
        // важно чтобы порядок значений в clearValues соответствовал порядку прикреплений
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color        = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues    = clearValues.data();

        // третий параметр определяет каким образом будут исполняться буферы команд
        // VK_SUBPASS_CONTENTS_INLINE - все комманды встроенны в главный буфер
        // и никакие вторичные буферы вызваны не будут
        // VK_SUBPASS_CONTENTS_INLINE - команды будут исполняться из вторичных буферов
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // второй параметр указывает что этот pipeline используется для графики
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
