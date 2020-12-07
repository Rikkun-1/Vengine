#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES // включает выравнивание по 16 бит для GLM структур
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

void createImage(VkPhysicalDevice      physicalDevice,
                 VkDevice              logicalDevice,
                 uint32_t              width,
                 uint32_t              height,
                 VkFormat              format,
                 VkImageTiling         tiling,
                 VkImageUsageFlags     usage,
                 VkMemoryPropertyFlags properties,
                 VkImage               &image,
                 VkDeviceMemory        &imageMemory);

void transitionImageLayout(VkDevice      logicalDevice,
                           VkCommandPool commandPool,
                           VkQueue       graphicsQueue,
                           VkImage       image,
                           VkFormat      format,
                           VkImageLayout oldLayout,
                           VkImageLayout newLayout);

void copyBufferToImage(VkDevice      logicalDevice,
                       VkCommandPool commandPool,
                       VkQueue       graphicsQueue,
                       VkBuffer      buffer,
                       VkImage       image,
                       uint32_t      width,
                       uint32_t      height);

void createTextureImage(VkPhysicalDevice physicalDevice,
                        VkDevice         logicalDevice,
                        VkCommandPool    commandPool,
                        VkQueue          graphicsQueue,
                        VkImage          &textureImage,
                        VkDeviceMemory   &textureImageMemory);

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

