#include "buffer.h"

void createBuffer(VkPhysicalDevice      physicalDevice,
                  VkDevice              logicalDevice,
                  VkDeviceSize          size,
                  VkBufferUsageFlags    usage,
                  VkMemoryPropertyFlags properties,
                  VkBuffer              &buffer,
                  VkDeviceMemory        &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    // буфер может использоваться конкретным семейством или же быть
    // быть общим для нескольких
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice,
                                               memRequirements.memoryTypeBits,
                                               properties);

    if(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!");

    vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
}

void createVertexBuffer(VkPhysicalDevice          physicalDevice,
                        VkDevice                  logicalDevice,
                        const std::vector<Vertex> &vertices,
                        VkCommandPool             commandPool,
                        VkQueue                   graphicsQueue,
                        VkBuffer                  &vertexBuffer,
                        VkDeviceMemory            &vertexBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer        stagingBuffer;
    VkDeviceMemory    stagingBufferMemory;

    createBuffer(physicalDevice,
                 logicalDevice,
                 bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    createBuffer(physicalDevice,
                 logicalDevice,
                 bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 vertexBuffer,
                 vertexBufferMemory);

    copyBuffer(logicalDevice, 
               commandPool, 
               graphicsQueue, 
               stagingBuffer, 
               vertexBuffer, 
               bufferSize);

    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void createIndexBuffer(VkPhysicalDevice             physicalDevice,
                        VkDevice                    logicalDevice,
                        const std::vector<uint32_t> &indices,
                        VkCommandPool               commandPool,
                        VkQueue                     graphicsQueue,
                        VkBuffer                    &indexBuffer,
                        VkDeviceMemory              &indexBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer        stagingBuffer;
    VkDeviceMemory    stagingBufferMemory;

    createBuffer(physicalDevice,
                 logicalDevice,
                 bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    createBuffer(physicalDevice,
                 logicalDevice,
                 bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 indexBuffer,
                 indexBufferMemory);

    copyBuffer(logicalDevice,
               commandPool,
               graphicsQueue,
               stagingBuffer,
               indexBuffer,
               bufferSize);

    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void createUniformBuffers(VkPhysicalDevice            physicalDevice,
                          VkDevice                    logicalDevice,
                          std::vector<VkBuffer>       &uniformBuffers,
                          std::vector<VkDeviceMemory> &uniformBuffersMemory,
                          int                         amount)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(amount);
    uniformBuffersMemory.resize(amount);

    for(size_t i = 0; i < amount; i++)
    {
        createBuffer(physicalDevice,
                     logicalDevice,
                     bufferSize, 
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                     uniformBuffers[i], 
                     uniformBuffersMemory[i]);
    }
}

void updateUniformBuffer(VkDevice                    logicalDevice,
                         uint32_t                    currentImage,
                         VkExtent2D                  swapChainExtent,
                         std::vector<VkDeviceMemory> &uniformBuffersMemory)
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

    void *data;
    vkMapMemory(logicalDevice, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(logicalDevice, uniformBuffersMemory[currentImage]);
}

void copyBuffer(VkDevice      logicalDevice,
                VkCommandPool commandPool,
                VkQueue       graphicsQueue,
                VkBuffer      srcBuffer,
                VkBuffer      dstBuffer,
                VkDeviceSize  size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice,
                                                            commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer,
                          logicalDevice,
                          commandPool,
                          graphicsQueue);
}

uint32_t findMemoryType(VkPhysicalDevice      physicalDevice,
                        uint32_t              typeFilter,
                        VkMemoryPropertyFlags properties)
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

