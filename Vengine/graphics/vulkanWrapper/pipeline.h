#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include <iostream>

#include "vertex.h"
#include "shaderModule.h"
#include "device.h"


struct PipelineFixedFunctions
{
    VkPipelineVertexInputStateCreateInfo         vertexInput;
    VkPipelineInputAssemblyStateCreateInfo       inputAssembly;
    VkViewport                                   viewport;       
    VkRect2D                                     scissor;
    VkPipelineViewportStateCreateInfo            viewportState;
    VkPipelineRasterizationStateCreateInfo       rasterizer;
    VkPipelineMultisampleStateCreateInfo         multisampling;
    VkPipelineDepthStencilStateCreateInfo        depthStencil;
    VkPipelineColorBlendAttachmentState          colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo          colorBlending;
    VkPipelineDynamicStateCreateInfo             dynamicState;

    void setup(const VkExtent2D &swapChainExtent);
    void setupViewPortAndScissor(const VkExtent2D  &swapChainExtent);

private:
    void setupVertexInputDescriptions();
    void setupAssemblyStateInfo      ();
    void setupViewPortStateInfo      ();
    void setupRasterizerStateInfo    ();
    void setupMultisamplingStateInfo ();
    void setupDepthStencilStateInfo  ();
    void setupColorAttachmentState   ();
    void setupColorBlendStateInfo    ();
    void setupDynamicStates          ();
};

VkPipeline createGraphicsPipeline(const LogicalDevice         &device,
                                  VkExtent2D                  &swapChainExtent,
                                  PipelineFixedFunctions      &fixedFunctions,
                                  VkRenderPass                renderPass,
                                  const ShaderModule          &vertexShader,
                                  const ShaderModule          &fragmentShader,
                                  const VkDescriptorSetLayout &descriptorSetLayout,
                                  const VkPipelineLayout      &pipelineLayout);