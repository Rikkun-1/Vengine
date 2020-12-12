#include "buffer.h"

#include <chrono>
#include <stdexcept>

#include "commandBuffer.h"


static uint32_t findMemoryType(VkPhysicalDevice       physicalDevice,
                               uint32_t               typeFilter,
                               VkMemoryPropertyFlags  properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        // проверяем что бит, отвечающий за доступность определенного типа памяти
        // true и проверяем что этот бит равен тому что мы запрашивали
        if((typeFilter & (1 << i)) &&
           // проверяем массив memoryTypes на наличие типа памяти,
           // который поддерживает все нужные нам возможности
           // например доступность этой памяти из CPU
           (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

    
Buffer::Buffer()
{
    this->device = VK_NULL_HANDLE;
    this->handle = VK_NULL_HANDLE;
    this->memory = VK_NULL_HANDLE;
    this->size   = 0;
}

Buffer::Buffer(const LogicalDevice *device)
{
    Buffer();
    this->device = device;
}

void Buffer::setDevice(const LogicalDevice *device)
{
     this->device = device;
}

void Buffer::create(VkDeviceSize           size,
                    VkBufferUsageFlags     usage,
                    VkMemoryPropertyFlags  properties)
{
    createBuffer(size, usage);
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->handle, this->handle, &memRequirements);
    allocateMemory(memRequirements, properties);
    vkBindBufferMemory(device->handle, this->handle, this->memory, 0);
}

void Buffer::destroy()
{
    if(handle)
    {
        vkDestroyBuffer(device->handle, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    if(memory)
    {
        vkFreeMemory(device->handle, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
}

void Buffer::createBuffer(VkDeviceSize        size,
                          VkBufferUsageFlags  usage) 
{
    this->size = size;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size  = size;
    bufferInfo.usage = usage;
    // буфер может использоваться конкретным семейством или же быть
    // быть общим для нескольких
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device->handle, &bufferInfo, nullptr, &this->handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex buffer!");
}

void Buffer::allocateMemory(const VkMemoryRequirements &memRequirements,
                            VkMemoryPropertyFlags       properties)
{
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(device->physicalDevice,
                                               memRequirements.memoryTypeBits,
                                               properties);

    if(vkAllocateMemory(device->handle, &allocInfo, nullptr, &this->memory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!");
}

void Buffer::mapMemory(VkDeviceSize  dataSize,
                       const void   *data)
{
    void *destination;
    vkMapMemory(this->device->handle, this->memory, 0, dataSize, 0, &destination);
    memcpy(destination, data, (size_t) dataSize);
    vkUnmapMemory(this->device->handle, this->memory);
}


static void copyBuffer(CommandPool          &commandPool,
                       Buffer               srcBuffer,
                       Buffer               dstBuffer,
                       VkDeviceSize         size)
{
    VkCommandBuffer commandBuffer;
    commandBuffer = beginSingleTimeCommands(commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, 
                    srcBuffer.handle, 
                    dstBuffer.handle, 
                    1, &copyRegion);

    endSingleTimeCommands(commandPool, commandBuffer);
}

static void setupAsStagingBuffer(VkDeviceSize bufferSize,
                                 Buffer       &buffer)
{
    buffer.create(bufferSize, 
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

static void setupAsUniformBuffer(VkDeviceSize bufferSize,
                                 Buffer       &buffer)
{
     buffer.create(bufferSize,
                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}


static void transferBufferToGPU(CommandPool         &commandPool,
                                Buffer              &srcBuffer,
                                Buffer              &gpuBuffer)
{
    gpuBuffer.setDevice(commandPool.device);
    gpuBuffer.create(srcBuffer.size,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(commandPool, 
               srcBuffer, 
               gpuBuffer, 
               srcBuffer.size);
}


static void transferDataToGPU(CommandPool         &commandPool,
                              VkDeviceSize         dataSize,
                              const void          *data,
                              Buffer              &bufferOnGpu)
{
    Buffer stagingBuffer(commandPool.device);
    setupAsStagingBuffer(dataSize, stagingBuffer);

    stagingBuffer.mapMemory(dataSize, data);

    transferBufferToGPU(commandPool, stagingBuffer, bufferOnGpu);

    stagingBuffer.destroy();
}


void createStagingBuffer(VkDeviceSize bufferSize,
                         Buffer       &buffer)
{
    setupAsStagingBuffer(bufferSize, buffer);
}


void createVertexBuffer(CommandPool               &commandPool,
                        const std::vector<Vertex> &vertices,
                        Buffer                    &vertexBuffer)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    transferDataToGPU(commandPool,
                      bufferSize,
                      vertices.data(),
                      vertexBuffer);
}


void createIndexBuffer(CommandPool                 &commandPool,
                       const std::vector<uint32_t> &indices,
                       Buffer                      &indexBuffer)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    transferDataToGPU(commandPool,
                      bufferSize,
                      indices.data(),
                      indexBuffer);
}


void createUniformBuffers(const LogicalDevice         &device,
                          std::vector<Buffer>         &uniformBuffers,
                          int                         amount)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(amount);

    for(size_t i = 0; i < amount; i++)
    {
        uniformBuffers[i].device = &device;
        setupAsUniformBuffer(bufferSize, uniformBuffers[i]);
    }
}


void updateUniformBuffer(VkDevice                    logicalDevice,
                         uint32_t                    currentImage,
                         VkExtent2D                  swapChainExtent,
                         std::vector<Buffer>         &uniformBuffers)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f),              // текущая трансформация
                            time * glm::radians(90.0f),   // угол
                            glm::vec3(0.0f, 1.0f, 1.0f)); // ось вращения

    ubo.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), // кординаты точки зрения
                           glm::vec3(0.0f, 0.0f, 0.0f), // координаты центра мира
                           glm::vec3(0.0f, 0.0f, 1.0f));// ось направленная вверх

    ubo.proj = glm::perspective(glm::radians(45.0f), // вертикальный угол обзора
                                // соотношение сторон
                                swapChainExtent.width / (float) swapChainExtent.height,
                                0.1f,   // ближайшая дистанция 
                                100.0f); // дальнейшая дистанция 
    // GLM была изначально создана для OpenGL, где ось Y перевернута
    // простейший способ компенсировать это - инвертировать Y компоненту
    // коэффициента масштабирования
    ubo.proj[1][1] *= -1;

    uniformBuffers[currentImage].mapMemory(sizeof(ubo), &ubo);
}



