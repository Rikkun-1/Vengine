#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES // включает выравнивание по 16 бит для GLM структур
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <vulkan/vulkan.h>

#include <vector>

#include "Vertex.h"
#include "commandBuffer.h"
#include "device.h"

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

    Buffer() = default;

    Buffer(const LogicalDevice &logicalDevice);

    void create(VkDeviceSize          size, 
                VkBufferUsageFlags    usage, 
                VkMemoryPropertyFlags properties);
    
    void mapMemory(VkDeviceSize dataSize, const void *data);

    void destroy();

private:
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    void allocateMemory(VkMemoryPropertyFlags properties);
};


void copyBuffer(const LogicalDevice  &device,
                VkCommandPool        commandPool,
                Buffer               srcBuffer,
                Buffer               dstBuffer,
                VkDeviceSize         size);


void createVertexBuffer(const LogicalDevice       &device,
                        const std::vector<Vertex> &vertices,
                        VkCommandPool             commandPool,
                        Buffer                    &vertexBuffer);


void createIndexBuffer(const LogicalDevice         &device,
                       const std::vector<uint32_t> &indices,
                       VkCommandPool               commandPool,
                       Buffer                      &indexBuffer);


void createUniformBuffers(const LogicalDevice         &device,
                          std::vector<Buffer>         &uniformBuffers,
                          int                         amount);


void updateUniformBuffer(VkDevice                    logicalDevice,
                         uint32_t                    currentImage,
                         VkExtent2D                  swapChainExtent,
                         std::vector<Buffer>         &uniformBuffers);
