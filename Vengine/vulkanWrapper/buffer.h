#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>

#include "Vertex.h"

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

void copyBuffer(const VkDevice	&logicalDevice,
				VkCommandPool	&commandPool,
				VkQueue			&graphicsQueue,
				VkBuffer		srcBuffer,
				VkBuffer		dstBuffer,
				VkDeviceSize	size);

uint32_t findMemoryType(const VkPhysicalDevice &physicalDevice,
						uint32_t				typeFilter,
						VkMemoryPropertyFlags	properties);