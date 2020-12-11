#include "CommandPool.h"


void CommandPool::allocateCommandBuffers(uint32_t         amount,
                                         VkCommandBuffer  *commandBuffers)  
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = this->handle;

    // ���� ����� �������� ��������� ��� ���������
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY - ����� �������� ���������.
    // ��� ����� ��������� � ������� ��� ����������, �� ��� ������ �������� �� ������ �������
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY - ����� �������� ���������. �� ����� ����
    // ��������� � ������� ��������, �� ����� ���� ������ �� ��������� �������
    // �� ��������� ������ ����� ������� ����������������� ������� ����� �������� �� 
    // �� �������� ������ � ������ �������
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = amount;

    if(vkAllocateCommandBuffers(this->device->handle, &allocInfo, commandBuffers) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");
}


CommandPool::CommandPool(const LogicalDevice  &device) 
{
    this->handle           = VK_NULL_HANDLE;;
    this->queueFamilyIndex = device.familyIndices.graphicsFamily.value();
}


void CommandPool::create() 
{
    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // ���������� ��������� ������� ���������� ����� �� ���������� �� � ���� �� �������� ��������
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    // ��������� �����:
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT - �� ������� ��� ���� ����� ����� ����� ����� �����������
    // (������ ����� �������� ���� ��������� ���������� ��������� ������)
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT - ��������� ��� �������������� ������ 
    // ������ ��� ������. ��� ���� ��� ����� ����� ��������� ������������
    poolInfo.flags = 0; // Optional

    if(vkCreateCommandPool(this->device->handle, &poolInfo, nullptr, &this->handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
}


void CommandPool::destroy()
{
    vkDestroyCommandPool(this->device->handle, this->handle, nullptr);
}