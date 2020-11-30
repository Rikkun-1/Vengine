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
						VkDeviceSize				&bufferSize,
						const std::vector<Vertex>	&vertices,
						VkBuffer					&vertexBuffer,
						VkDeviceMemory				&vertexBufferMemory);

uint32_t findMemoryType(const VkPhysicalDevice &physicalDevice,
						uint32_t				typeFilter,
						VkMemoryPropertyFlags	properties);