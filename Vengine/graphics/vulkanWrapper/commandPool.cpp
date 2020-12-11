#include "CommandPool.h"

void CommandPool::freeCommandBuffers(int              amount,
                                     VkCommandBuffer *commandBuffers)
{
     vkFreeCommandBuffers(device->handle, 
                          handle, 
                          static_cast<uint32_t>(amount), 
                          commandBuffers);
}

void CommandPool::allocateCommandBuffers(int              amount,
                                         VkCommandBuffer  *commandBuffers)  
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = handle;

    // Этот буфер является первичным или вторичным
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY - буфер является первичным.
    // его можно отправить в очередь для исполнения, но его нельзя вызывать из других буферов
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY - буфер является вторичным. Не может быть
    // отправлен в очередь напрямую, но может быть вызван из первичных буферов
    // во вторичном буфере можно описать частоиспользуемые команды чтобы вызывать их 
    // из главного буфера в нужные моменты
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(amount);

    if(vkAllocateCommandBuffers(device->handle, &allocInfo, commandBuffers) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");
}


CommandPool::CommandPool(LogicalDevice *device = VK_NULL_HANDLE)
{
    handle           = VK_NULL_HANDLE;;
    device           = device;
    queueFamilyIndex = device->familyIndices.graphicsFamily.value();
}

void CommandPool::create() 
{
    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // исполнение командных буферов происходит когда мы отправляем их в одно из семейств очередей
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    // Возможные флаги:
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT - мы говорим что этот буфер будет очень часто перезаписан
    // (вулкан может поменять свое поведение касательно выделения памяти)
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT - позволяет нам перезаписывать только 
    // нужные нам буферы. Без него все буфеы будут очищаться одновременно
    poolInfo.flags = 0; // Optional

    if(vkCreateCommandPool(this->device->handle, &poolInfo, nullptr, &this->handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
}

void CommandPool::setDevice(LogicalDevice *device)
{
    this->device           = device;
    this->queueFamilyIndex = device->familyIndices.graphicsFamily.value();
}

void CommandPool::destroy()
{
    vkDestroyCommandPool(this->device->handle, this->handle, nullptr);
}