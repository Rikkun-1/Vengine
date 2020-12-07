#include "buffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void createImage(const VkPhysicalDevice &physicalDevice,
				 const VkDevice			&logicalDevice,
				 uint32_t				width, 
				 uint32_t				height, 
				 VkFormat				format, 
				 VkImageTiling			tiling, 
				 VkImageUsageFlags		usage, 
				 VkMemoryPropertyFlags	properties,
				 VkImage				&image, 
				 VkDeviceMemory			&imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType		= VK_IMAGE_TYPE_2D;
	imageInfo.extent.width	= width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth	= 1;
	imageInfo.mipLevels		= 1;
	imageInfo.arrayLayers	= 1;
	imageInfo.format		= format;
	imageInfo.tiling		= tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage			= usage;
	imageInfo.samples		= VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;

	if(vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("failed to create image!");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(physicalDevice,
											   memRequirements.memoryTypeBits, 
											   properties);

	if(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate image memory!");

	vkBindImageMemory(logicalDevice, image, imageMemory, 0);
}

void transitionImageLayout(VkDevice		 &logicalDevice,
						   VkCommandPool commandPool,
						   VkQueue		 graphicsQueue, 
						   VkImage		 image,
						   VkFormat		 format, 
						   VkImageLayout oldLayout, 
						   VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice,
															commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;

	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
	   newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
			newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
		throw std::invalid_argument("unsupported layout transition!");

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(commandBuffer,
						  logicalDevice,
						  commandPool,
						  graphicsQueue);
}

void copyBufferToImage(VkDevice		 &logicalDevice,
					   VkCommandPool commandPool,
					   VkQueue		 graphicsQueue,
					   VkBuffer		 buffer, 
					   VkImage		 image, 
					   uint32_t		 width,
					   uint32_t		 height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice,
															commandPool);

	VkBufferImageCopy region{};
	region.bufferOffset		 = 0;
	region.bufferRowLength	 = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(commandBuffer,
						  logicalDevice,
						  commandPool,
						  graphicsQueue);
}

void createTextureImage(const VkPhysicalDevice  &physicalDevice,
						VkDevice				&logicalDevice,
						VkCommandPool			&commandPool,
						VkQueue					graphicsQueue,
						VkImage					&textureImage,
						VkDeviceMemory			&textureImageMemory)
{
	int texWidth,
		texHeight,
		texChannels;

	stbi_uc *pixels = stbi_load("textures/texture.jpg",
								&texWidth,
								&texHeight,
								&texChannels,
								STBI_rgb_alpha);

	// последний аргумент - кол-во байт на пиксель. 4 в случае с форматом STBI_rgb_alpha;
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if(!pixels)
		throw std::runtime_error("failed to load texture image!");

	VkBuffer       stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(physicalDevice,
				 logicalDevice,
				 imageSize,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 stagingBuffer,
				 stagingBufferMemory);

	void *data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage(physicalDevice,
				logicalDevice,
				texWidth, 
				texHeight, 
				VK_FORMAT_R8G8B8A8_SRGB, 
				VK_IMAGE_TILING_OPTIMAL, 
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
				VK_IMAGE_USAGE_SAMPLED_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				textureImage, 
				textureImageMemory);

	transitionImageLayout(logicalDevice,
						  commandPool,
						  graphicsQueue,
						  textureImage,
						  VK_FORMAT_R8G8B8A8_SRGB,
						  VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copyBufferToImage(logicalDevice,
					  commandPool,
					  graphicsQueue,
					  stagingBuffer,
					  textureImage,
					  static_cast<uint32_t>(texWidth),
					  static_cast<uint32_t>(texHeight));

	transitionImageLayout(logicalDevice,
						  commandPool,
						  graphicsQueue,
						  textureImage, 
						  VK_FORMAT_R8G8B8A8_SRGB, 
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}


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
	// буфер может использоватьс€ конкретным семейством или же быть
	// быть общим дл€ нескольких
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
						const std::vector<Vertex>	&vertices,
						VkCommandPool				&commandPool,
						VkQueue						&graphicsQueue,
						VkBuffer					&vertexBuffer,
						VkDeviceMemory				&vertexBufferMemory)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer		stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;

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

void createIndexBuffer(const VkPhysicalDevice		&physicalDevice,
						const VkDevice				&logicalDevice,
						const std::vector<uint32_t>	&indices,
						VkCommandPool				&commandPool,
						VkQueue						&graphicsQueue,
						VkBuffer					&indexBuffer,
						VkDeviceMemory				&indexBufferMemory)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer		stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;

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

void createUniformBuffers(const VkPhysicalDevice		&physicalDevice,
						  const VkDevice				&logicalDevice,
						  std::vector<VkBuffer>			&uniformBuffers,
						  std::vector<VkDeviceMemory>	&uniformBuffersMemory,
						  int							amount)
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

void updateUniformBuffer(const VkDevice					&logicalDevice,
						 uint32_t						currentImage,
						 VkExtent2D						&swapChainExtent,
						 std::vector<VkDeviceMemory>	&uniformBuffersMemory)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f),			  // текуща€ трансформаци€
							time * glm::radians(90.0f),	  // угол
							glm::vec3(0.0f, 0.0f, 1.0f)); // ось вращени€

	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), // кординаты точки зрени€
						   glm::vec3(0.0f, 0.0f, 0.0f), // координаты центра мира
						   glm::vec3(0.0f, 0.0f, 1.0f));// ось направленна€ вверх

	ubo.proj = glm::perspective(glm::radians(45.0f), // вертикальный угол обзора
								// соотношение сторон
								swapChainExtent.width / (float) swapChainExtent.height,
								0.1f,   // ближайша€ дистанци€ 
								100.0f); // дальнейша€ дистанци€ 
	// GLM была изначально создана дл€ OpenGL, где ось Y перевернута
	// простейший способ компенсировать это - инвертировать Y компоненту
	// коэффициента масштабировани€
	ubo.proj[1][1] *= -1;

	void *data;
	vkMapMemory(logicalDevice, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(logicalDevice, uniformBuffersMemory[currentImage]);
}


void copyBuffer(const VkDevice	&logicalDevice,
				VkCommandPool	&commandPool,
				VkQueue			&graphicsQueue,
				VkBuffer		srcBuffer,
				VkBuffer		dstBuffer,
				VkDeviceSize	size)
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

uint32_t findMemoryType(const VkPhysicalDevice		&physicalDevice,
						uint32_t					typeFilter,
						VkMemoryPropertyFlags		properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		// провер€ем что бит, отвечающий за доступность определенного типа пам€ти
		// true и провер€ем что этот бит равен тому что мы запрашивали
		if((typeFilter & (1 << i)) &&
		   // провер€ем массив memoryTypes на наличие типа пам€ти,
		   // который поддерживает все нужные нам возможности
		   // например доступность этой пам€ти из CPU
		   (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

