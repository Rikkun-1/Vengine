#include "descriptorSets.h"

#include <stdexcept>
#include <array>

#include "buffer.h"

static void allocateDescriptorSets(VkDevice                      logicalDevice,
                                   VkDescriptorPool              descriptorPool,
                                   VkDescriptorSetLayout        &descriptorSetLayout,
                                   std::vector<VkDescriptorSet> &descriptorSets,
                                   int                           amount)
{
    std::vector<VkDescriptorSetLayout> layouts(amount, descriptorSetLayout);
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(amount);
    allocInfo.pSetLayouts        = layouts.data();

    descriptorSets.resize(amount);
    if(vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");
}


static void setupDescriptorBufferInfo(VkBuffer                 buffer,
                                      uint32_t                 offset,
                                      uint32_t                 range,                  
                                      VkDescriptorBufferInfo  &bufferInfo)
{
    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    // ���� �� �������� ������ ��� �������������� ����� �������, 
    // �� �� ����� ������������ VK_WHOLE_SIZE ��� range
    bufferInfo.range = range;
}


static void setupDescriptorImageInfo(VkImageLayout          layout,
                                     VkImageView            textureImageView,
                                     VkSampler              textureSampler,
                                     VkDescriptorImageInfo  &imageInfo)
{
    imageInfo.imageLayout = layout;
    imageInfo.imageView   = textureImageView;
    imageInfo.sampler     = textureSampler;
}


void createDescriptorPool(VkDevice                   logicalDevice,
                          const std::vector<VkImage> swapChainImages,
                          VkDescriptorPool           &descriptorPool)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

    poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());


    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = static_cast<uint32_t>(swapChainImages.size());


    if(vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool!");
}


void createDescriptorSets(VkDevice                     logicalDevice,
                          VkDescriptorPool             descriptorPool,
                          VkDescriptorSetLayout        descriptorSetLayout,
                          std::vector<VkDescriptorSet> &descriptorSets,
                          std::vector<VkBuffer>        &uniformBuffers,
                          VkImageView                  textureImageView,
                          VkSampler                    textureSampler,
                          int                          amount)
{
    allocateDescriptorSets(logicalDevice,
                           descriptorPool,
                           descriptorSetLayout,
                           descriptorSets,
                           amount);

    for(size_t i = 0; i < amount; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        setupDescriptorBufferInfo(uniformBuffers[i], 
                                  0, 
                                  sizeof(UniformBufferObject), 
                                  bufferInfo);

        VkDescriptorImageInfo imageInfo{};
        setupDescriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 textureImageView,
                                 textureSampler,
                                 imageInfo);

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet          = descriptorSets[i];
        descriptorWrites[0].dstBinding      = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;

        descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet          = descriptorSets[i];
        descriptorWrites[1].dstBinding      = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(logicalDevice, 
                               static_cast<uint32_t>(descriptorWrites.size()), 
                               descriptorWrites.data(), 
                               0, 
                               nullptr);
    }
}
