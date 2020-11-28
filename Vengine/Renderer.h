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

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence>	 inFlightFences;
	std::vector<VkFence>	 imagesInFlight;

	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame			   = 0;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void drawFrame();
	void cleanup();
};

