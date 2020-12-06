#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>

#include "Vertex.h"



struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

void createBuffer(const VkPhysicalDevice	&physicalDevice,
				  const VkDevice			&logicalDevice,
				  VkDeviceSize				size,
				  VkBufferUsageFlags		usage,
				  VkMemoryPropertyFlags		properties,
				  VkBuffer					&buffer,
				  VkDeviceMemory			&bufferMemory);


void createVertexBuffer(const VkPhysicalDevice		&physicalDevice,
						const VkDevice				&logicalDevice,
						const std::vector<Vertex>	&vertices,
						VkCommandPool				&commandPool,
						VkQueue						&graphicsQueue,
						VkBuffer					&vertexBuffer,
						VkDeviceMemory				&vertexBufferMemory);

void createIndexBuffer(const VkPhysicalDevice		&physicalDevice,
						const VkDevice				&logicalDevice,
						const std::vector<uint32_t>	&indices,
						VkCommandPool				&commandPool,
						VkQueue						&graphicsQueue,
						VkBuffer					&indexBuffer,
						VkDeviceMemory				&indexBufferMemory);

void createUniformBuffers(const VkPhysicalDevice		&physicalDevice,
						  const VkDevice				&logicalDevice,
						  std::vector<VkBuffer>			&uniformBuffers,
						  std::vector<VkDeviceMemory>	&uniformBuffersMemory,
						  int							amount);

void updateUniformBuffer(const VkDevice					&logicalDevice,
						 uint32_t						currentImage,
						 VkExtent2D						&swapChainExtent,
						 std::vector<VkDeviceMemory>	&uniformBuffersMemory);


void copyBuffer(const VkDevice	&logicalDevice,
				VkCommandPool	&commandPool,
				VkQueue			&graphicsQueue,
				VkBuffer		srcBuffer,
				VkBuffer		dstBuffer,
				VkDeviceSize	size);

uint32_t findMemoryType(const VkPhysicalDevice &physicalDevice,
						uint32_t				typeFilter,
						VkMemoryPropertyFlags	properties);