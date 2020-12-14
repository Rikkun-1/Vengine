#include "descriptorSet.h"

///////////////////////// PUBLIC BEG //////////////////////////////

static void allocateDescriptorSets(VkDevice                      logicalDevice,
                                   VkDescriptorPool              descriptorPool,
                                   VkDescriptorSetLayout        &descriptorSetLayout,
                                   std::vector<VkDescriptorSet> &descriptorSets,
                                   uint32_t                      amount)
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
    // если мы намерены каждый раз перезаписывать буфер целиком, 
    // то мы можем использовать VK_WHOLE_SIZE для range
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


void createDescriptorSets(const LogicalDevice          &device,
                          VkDescriptorPool             descriptorPool,
                          VkDescriptorSetLayout        descriptorSetLayout,
                          std::vector<VkDescriptorSet> &descriptorSets,
                          std::vector<Buffer>          &uniformBuffers,
                          VkImageView                  textureImageView,
                          VkSampler                    textureSampler,
                          uint32_t                     amount)
{
    allocateDescriptorSets(device.handle,
                           descriptorPool,
                           descriptorSetLayout,
                           descriptorSets,
                           amount);

    for(size_t i = 0; i < amount; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        setupDescriptorBufferInfo(uniformBuffers[i].handle, 
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

        vkUpdateDescriptorSets(device.handle,
                               static_cast<uint32_t>(descriptorWrites.size()), 
                               descriptorWrites.data(), 
                               0, 
                               nullptr);
    }
}

///////////////////////// PUBLIC END /////////////////////////////