#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkanWrapper/vulkanWrapper.h"
#include "vkSettings.h"

using std::vector;

class Renderer
{
public:
	vkSettings settings;

	void run();

private:
	GLFWwindow				*pWindow;
	VkInstance				 instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR			 surface;

	VkPhysicalDevice		 physicalDevice;
	VkDevice				 logicalDevice;

	VkQueue					 graphicsQueue;
	VkQueue					 presentationQueue;

	VkSwapchainKHR			 swapChain;
	vector<VkImage>		     swapChainImages;
	VkFormat				 swapChainImageFormat;
	VkExtent2D				 swapChainExtent;

	vector<VkImageView>		 swapChainImageViews;

	VkRenderPass			 renderPass;

	VkPipelineLayout		 pipelineLayout;
	VkPipeline				 graphicsPipeline;

	vector<VkFramebuffer>	 swapChainFramebuffers;

	VkCommandPool			 commandPool;
	vector<VkCommandBuffer>  commandBuffers;

	VkSemaphore				 imageAvailableSemaphore;
	VkSemaphore				 renderFinishedSemaphore;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void drawFrame();
	void createSemaphores();
	void cleanup();
};

