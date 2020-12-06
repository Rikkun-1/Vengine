#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>

#include "setupValidationLayers.h"
#include "extensionsCheck.h"
#include "setupDevices.h"

#include "setupSwapchain.h"
#include "buffer.h"

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else 
	const bool enableValidationLayers = true;
#endif

#include "vulkanWrapper.h"


void createInstance(const std::vector<const char *> &validationLayers,
					const std::vector<const char *> &instanceExtensions,
					VkInstance &instance);


void setupDebugMessenger(VkInstance &instance, VkDebugUtilsMessengerEXT &debugMessenger);


VkPhysicalDevice pickPhysicalDevice(const VkInstance   &instance,
									const VkSurfaceKHR &surface,
									const std::vector<const char *> &requiredExtenisons);


void createLogicalDevice(const VkPhysicalDevice			 &physicalDevice,
						 const VkSurfaceKHR			     &surface,
						 const std::vector<const char *> &requiredExtenisons,
						 VkDevice						 &logicalDevice,
						 VkQueue						 &graphicsQueue,
						 VkQueue						 &presentQueue);


void createSwapChain(GLFWwindow				*pWindow,
					 const VkPhysicalDevice &physicalDevice,
					 const VkDevice			&logicalDevice,
					 const VkSurfaceKHR		&surface,
					 VkSwapchainKHR			&swapChain,
					 std::vector<VkImage>	&swapChainImages,
					 VkFormat				&swapChainImageFormat,
					 VkExtent2D				&swapChainExtent);


void createImageViews(const VkDevice			 &logicalDevice,
					  const VkFormat			 &swapChainImageFormat,
					  const std::vector<VkImage> &swapChainImages,
					  std::vector<VkImageView>	 &swapChainImageViews);

///////////////////////////////////////////////////////////

void createRenderPass(const VkDevice &logicalDevice,
					  const VkFormat &swapChainImageFormat,
					  VkRenderPass   &renderPass);

///////////////////////////////////////////////////////////

VkShaderModule createShaderModule(const VkDevice &logicalDevice,
								  const std::vector<char> &code);


void createGraphicsPipeline(const VkDevice			&logicalDevice,
							const VkExtent2D		&swapChainExtent,
							VkRenderPass			&renderPass,
							VkPipelineLayout		&pipelineLayout,
							VkPipeline				&graphicsPipeline,
							VkDescriptorSetLayout	&descriptorSetLayout);


static std::vector<char> readFile(const std::string &filename);

///////////////////////////////////////////////////////////

void createFramebuffers(const VkDevice					&logicalDevice,
						const VkRenderPass				&renderPass,
						const VkExtent2D				&swapChainExtent,
						const std::vector<VkImageView>	&swapChainImageViews,
						std::vector<VkFramebuffer>		&swapChainFramebuffers);

///////////////////////////////////////////////////////////

void createCommandPool(const VkPhysicalDevice	&physicalDevice,
					   const VkDevice			&logicalDevice,
					   const VkSurfaceKHR		&surface, 
					   VkCommandPool			&commandPool);

void createCommandBuffers(const VkDevice				   &logicalDevice,
						  const VkExtent2D				   &swapChainExtent,
						  const std::vector<VkFramebuffer> &swapChainFramebuffers,
						  const VkPipeline				   &graphicsPipeline,
						  const VkRenderPass			   &renderPass,
						  VkBuffer						   &vertexBuffer,
						  VkBuffer						   &indexBuffer,
						  VkCommandPool					   &commandPool,
						  std::vector<VkCommandBuffer>	   &commandBuffers,
						  std::vector<VkDescriptorSet>	   &descriptorSets,
						  VkPipelineLayout				   &pipelineLayout,
						  int								indexBufferSize);

///////////////////////////////////////////////////////////

void createSyncObjects(const VkDevice &logicalDevice,
					   int MAX_FRAMES_IN_FLIGHT,
					   const std::vector<VkImage> &swapChainImages,
					   std::vector<VkSemaphore>   &imageAvailableSemaphores,
					   std::vector<VkSemaphore>   &renderFinishedSemaphores,
					   std::vector<VkFence>		  &inFlightFences,
					   std::vector<VkFence>		  &imagesInFlight);

//////////////////////////////////////////////////////////

void createDescriptorSetLayout(const VkDevice			&logicalDevice, 
							   VkDescriptorSetLayout	&descriptorSetLayout);

void createDescriptorPool(const VkDevice			 &logicalDevice,
						  const std::vector<VkImage> &swapChainImages,
						  VkDescriptorPool			 &descriptorPool);

void createDescriptorSets(const VkDevice			   &logicalDevice,
						  const std::vector<VkImage>   &swapChainImages,
						  VkDescriptorPool			   &descriptorPool,
						  VkDescriptorSetLayout		   &descriptorSetLayout,
						  std::vector<VkDescriptorSet> &descriptorSets,
						  std::vector<VkBuffer>		   &uniformBuffers);