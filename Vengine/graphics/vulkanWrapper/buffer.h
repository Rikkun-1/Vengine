#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES // включает выравнивание по 16 бит для GLM структур
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#include <vector>
#include "device.h"
#include "commandPool.h"
#include "vertex.h"


struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};


struct Buffer
{
    VkBuffer              handle;
    VkDeviceMemory        memory;
    VkDeviceSize          size;
    const LogicalDevice   *device;

    Buffer();

    Buffer(const LogicalDevice *logicalDevice);

    void create(VkDeviceSize          size, 
                VkBufferUsageFlags    usage, 
                VkMemoryPropertyFlags properties);
    
    void setDevice(const LogicalDevice *device);

    void mapMemory(VkDeviceSize dataSize, const void *data);
    
    void destroy();

protected:
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

    void allocateMemory(const VkMemoryRequirements &memRequirements,
                        VkMemoryPropertyFlags       properties);
};

void createStagingBuffer(VkDeviceSize  bufferSize,
                         Buffer        &buffer);


void createVertexBuffer(CommandPool               &commandPool,
                        const std::vector<Vertex> &vertices,
                        Buffer                    &vertexBuffer);


void createIndexBuffer(CommandPool                 &commandPool,
                       const std::vector<uint32_t> &indices,
                       Buffer                      &indexBuffer);


void createUniformBuffers(const LogicalDevice         &device,
                          std::vector<Buffer>         &uniformBuffers,
                          int                         amount);


void updateUniformBuffer(VkDevice                    logicalDevice,
                         uint32_t                    currentImage,
                         VkExtent2D                  swapChainExtent,
                         glm::vec3                   position,
                         glm::vec3                   rotation,
                         glm::vec3                   scale,
                         std::vector<Buffer>         &uniformBuffers);

