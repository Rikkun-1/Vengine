#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES // включает выравнивание по 16 бит для GLM структур
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>

#include "Vertex.h"
#include "commandBuffer.h"

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

void createBuffer(VkPhysicalDevice      physicalDevice,
                  VkDevice              logicalDevice,
                  VkDeviceSize          size,
                  VkBufferUsageFlags    usage,
                  VkMemoryPropertyFlags properties,
                  VkBuffer              &buffer,
                  VkDeviceMemory        &bufferMemory);

void createVertexBuffer(VkPhysicalDevice          physicalDevice,
                        VkDevice                  logicalDevice,
                        const std::vector<Vertex> &vertices,
                        VkCommandPool             commandPool,
                        VkQueue                   graphicsQueue,
                        VkBuffer                  &vertexBuffer,
                        VkDeviceMemory            &vertexBufferMemory);

void createIndexBuffer(VkPhysicalDevice            physicalDevice,
                       VkDevice                    logicalDevice,
                       const std::vector<uint32_t> &indices,
                       VkCommandPool               commandPool,
                       VkQueue                     graphicsQueue,
                       VkBuffer                    &indexBuffer,
                       VkDeviceMemory              &indexBufferMemory);

void createUniformBuffers(VkPhysicalDevice            physicalDevice,
                          VkDevice                    logicalDevice,
                          std::vector<VkBuffer>       &uniformBuffers,
                          std::vector<VkDeviceMemory> &uniformBuffersMemory,
                          int                         amount);

void updateUniformBuffer(VkDevice                    logicalDevice,
                         uint32_t                    currentImage,
                         VkExtent2D                  swapChainExtent,
                         std::vector<VkDeviceMemory> &uniformBuffersMemory);

void copyBuffer(VkDevice      logicalDevice,
                VkCommandPool commandPool,
                VkQueue       graphicsQueue,
                VkBuffer      srcBuffer,
                VkBuffer      dstBuffer,
                VkDeviceSize  size);

uint32_t findMemoryType(VkPhysicalDevice      physicalDevice,
                        uint32_t              typeFilter,
                        VkMemoryPropertyFlags properties);

