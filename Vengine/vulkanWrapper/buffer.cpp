#include "buffer.h"

void createBuffer(const VkPhysicalDevice	&physicalDevice,
				  const VkDevice			&logicalDevice,
				  VkDeviceSize				size,
				  VkBufferUsageFlags		usage,
				  VkMemoryPropertyFlags		properties,
				  VkBuffer					&buffer,
				  VkDeviceMemory			&bufferMemory)
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

void createVertexBuffer(const VkPhysicalDevice		&physicalDevice,
						const VkDevice				&logicalDevice,
						VkDeviceSize				&bufferSize,
						const std::vector<Vertex>	&vertices,
						VkBuffer					&vertexBuffer,
						VkDeviceMemory				&vertexBufferMemory)
{
	createBuffer(physicalDevice,
				 logicalDevice,
				 bufferSize,
				 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 vertexBuffer,
				 vertexBufferMemory);

	void *data;
	vkMapMemory(logicalDevice, vertexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(logicalDevice, vertexBufferMemory);
}

uint32_t findMemoryType(const VkPhysicalDevice		&physicalDevice,
						uint32_t					typeFilter,
						VkMemoryPropertyFlags		properties)
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