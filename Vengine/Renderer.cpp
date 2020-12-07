#include "Renderer.h"

static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
	auto app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Renderer::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void Renderer::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	pWindow = glfwCreateWindow(settings.windowWidth, settings.windowHeight, 
							  settings.windowTitle, nullptr, nullptr);

	// ����������� � ���� ��������� �� ��������� ������ Renderer
	// ����� � framebufferResizeCallback ����� ������ � framebufferResized
	glfwSetWindowUserPointer(pWindow, this);
	glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);
}

void Renderer::initVulkan()
{
	createInstance(settings.validationLayers,
				   settings.instanceExtensions,
				   instance);

	setupDebugMessenger(instance, debugMessenger);

	if(glfwCreateWindowSurface(instance, pWindow, nullptr, &surface) != VK_SUCCESS)
		std::runtime_error("failed to create window surface");

	physicalDevice = pickPhysicalDevice(instance, 
										surface, 
										settings.deviceExtensions);

	createLogicalDevice(physicalDevice, 
						surface, 
						settings.deviceExtensions, 
						logicalDevice,
						graphicsQueue, 
						presentationQueue);

	createSwapChain(pWindow,
					physicalDevice,
					logicalDevice,
					surface,
					swapChain,
					swapChainImages,
					swapChainImageFormat,
					swapChainExtent);

	createImageViews(logicalDevice,
					 swapChainImageFormat,
					 swapChainImages,
					 swapChainImageViews);


	createRenderPass(logicalDevice,
					 swapChainImageFormat,
					 renderPass);

	createDescriptorSetLayout(logicalDevice,
							  descriptorSetLayout);

	createGraphicsPipeline(logicalDevice,
						   swapChainExtent,
						   renderPass,
						   pipelineLayout,
						   graphicsPipeline,
						   descriptorSetLayout);


	createFramebuffers(logicalDevice,
					   renderPass,
					   swapChainExtent,
					   swapChainImageViews,
					   swapChainFramebuffers);


	createCommandPool(physicalDevice,
					  logicalDevice,
					  surface,
					  commandPool);

	createTextureImage(physicalDevice,
					   logicalDevice,
					   commandPool,
					   graphicsQueue,
					   textureImage,
					   textureImageMemory);

	createVertexBuffer(physicalDevice,
					   logicalDevice,
					   vertices,
					   commandPool,
					   graphicsQueue,
					   vertexBuffer,
					   vertexBufferMemory);

	createIndexBuffer(physicalDevice,
					  logicalDevice,
					  indices,
					  commandPool,
					  graphicsQueue,
					  indexBuffer,
					  indexBufferMemory);

	createUniformBuffers(physicalDevice,
					     logicalDevice,
					     uniformBuffers,
					     uniformBuffersMemory,
						 swapChainImages.size());

	createDescriptorPool(logicalDevice, 
						 swapChainImages,
						 descriptorPool);

	createDescriptorSets(logicalDevice, 
						 swapChainImages,
						 descriptorPool,
						 descriptorSetLayout,
						 descriptorSets,
						 uniformBuffers);

	createCommandBuffers(logicalDevice,
						 swapChainExtent,
						 swapChainFramebuffers,
						 graphicsPipeline,
						 renderPass,
						 vertexBuffer,
						 indexBuffer,
						 commandPool,
						 commandBuffers,
						 descriptorSets,
						 pipelineLayout,
						 static_cast<uint32_t>(indices.size()));

	createSyncObjects(logicalDevice,
					  MAX_FRAMES_IN_FLIGHT,
					  swapChainImages,
					  imageAvailableSemaphores,
					  renderFinishedSemaphores,
					  inFlightFences,
					  imagesInFlight);
}

void Renderer::mainLoop()
{
	while(!glfwWindowShouldClose(pWindow))
	{
		glfwPollEvents();
		drawFrame();
	}

	// ������ ��� ����� �� �������� ����� �� ���� ����� ���������� ��������� ���������� ����������
	// ���������� ������ 
	vkDeviceWaitIdle(logicalDevice);
}

void Renderer::recreateSwapChain()
{

	/*
	//int width  = 0, 
	//	height = 0;
	//glfwGetFramebufferSize(pWindow, &width, &height);
	// ���� ������ ���� � ������ ������ �����������, �� glfwGetFramebufferSize ������ ����
	// ������ ������ � ������. ������� �� ���� ���� ������ ���� �� ������ ���������� ��������
	// ��� ���������� ����� ���� ���������� ������ ������
	//std::cout << width << "  " << height << std::endl;
	//while(width == 0 || height == 0)
	//{
	//	std::cout << width << " || " << height << std::endl;
	//	glfwGetFramebufferSize(pWindow, &width, &height);
	//	glfwWaitEvents();
	//}
	// �� ���������. ��������� �� ������� � ���� ��� ��� ��� ����������� GLFW ���������� 
	// �������� ���������� �������� ������� �����.
	*/

	vkDeviceWaitIdle(logicalDevice);

	cleanupSwapChain();

	createSwapChain(pWindow,
					physicalDevice,
					logicalDevice,
					surface,
					swapChain,
					swapChainImages,
					swapChainImageFormat,
					swapChainExtent);


	createImageViews(logicalDevice,
					 swapChainImageFormat,
					 swapChainImages,
					 swapChainImageViews);


	createRenderPass(logicalDevice,
					 swapChainImageFormat,
					 renderPass);

	createDescriptorSetLayout(logicalDevice,
							  descriptorSetLayout);

	createGraphicsPipeline(logicalDevice,
						   swapChainExtent,
						   renderPass,
						   pipelineLayout,
						   graphicsPipeline,
						   descriptorSetLayout);

	createFramebuffers(logicalDevice,
					   renderPass,
					   swapChainExtent,
					   swapChainImageViews,
					   swapChainFramebuffers);

	createUniformBuffers(physicalDevice,
						 logicalDevice,
						 uniformBuffers,
						 uniformBuffersMemory,
						 uniformBuffers.size());

	createDescriptorPool(logicalDevice,
						 swapChainImages,
						 descriptorPool);

	createDescriptorSets(logicalDevice,
						 swapChainImages,
						 descriptorPool,
						 descriptorSetLayout,
						 descriptorSets,
						 uniformBuffers);

	createCommandBuffers(logicalDevice,
						 swapChainExtent,
						 swapChainFramebuffers,
						 graphicsPipeline,
						 renderPass,
						 vertexBuffer,
						 indexBuffer,
						 commandPool,
						 commandBuffers,
						 descriptorSets,
						 pipelineLayout,
						 static_cast<uint32_t>(indices.size()));
}

void Renderer::drawFrame()
{
	// ������� ����� ��� ������ ������� � ���������� ���������
	// ������ ��������� ������ ������� � � ����������� �� 4-�� ��������� 
	// ���� ���� ���� ����� �����, ���� ����� ���
	// ��������� �������� ��� �� ��� ����� �� ���������� ����� � ������������
	// ������� ���������� �� ��������� ������� ���� ����� ������. � ���� ������ ��� ������ VK_TIMEOUT
	// ����� VK_SUCCESS
	// �������� UINT64_MAX �������� ��� ����������� �� ������� ���
	vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	// ������, ����������� ����� ����������� � swapChainImages 
	// ����� ���������
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(logicalDevice,
											swapChain,
											// ����� � ������������, ������ �����������
											// ����� ����� ���������. �������� UINT64_MAX - ��������� �������
											UINT64_MAX,
											// �������, ������� ����� ������� ���������� ����� ����������� ����� ������ ��� �������������
											imageAvailableSemaphores[currentFrame],
											// ����������, �����. ����� ������������ ��� ������������. 
											// �� �� ���������� ������ �������
											VK_NULL_HANDLE,
											// � imageIndex ����� ������� ������ �����������, �������
											// ����� ���������
											&imageIndex);
	
	// ���� vkAcquireNextImageKHR ������ VK_ERROR_OUT_OF_DATE_KHR
	// �� ��� ������ ��� ������� ������ ����������� � ������� ������ ����� �� ������������ 
	// ����������� � ������� ��������� ������� ������
	// ����� ����� ��������� �������� ��� ��������� ������� ����
	// VK_SUBOPTIMAL_KHR �������� ��� ����������� ��� ��� ����� �������������� ��� ���������,
	// �� ��� ����� �� �������� �������������, ��������� ������������ � ������������ �� ������
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// �� ����������� ������� ������ ����� ��� ����� ����� ����������� � ������������ ��� ������
		recreateSwapChain();
		// �� �� �������� ����������� ��� ��������� �������� �����. 
		// ��� ����������� ���������� ��� �� ����� �����������
		return;
	}
	else if(result != VK_SUCCESS)
		throw std::runtime_error("failed to acquire swap chain image!");

	// ���� ���������� ���� ��� ��� ���������� ��� �����������, �� �� ���� �� ��� ��� ���� 
	// ��� �� �����������
	if(imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	
	// ��������� �����������, ���������� �� ������� ������ � �������, ������� ������
	// ���������� ����� ��������� � ��� ����������� ����� ��������
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	updateUniformBuffer(logicalDevice,
						imageIndex,
						swapChainExtent,
						uniformBuffersMemory);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// ��������� ��������, ������ ���������� ��������� �� ������
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};

	// ��������� �� ����� ����� ������ �� ������� ��������
	// ������ ������� ����������� �� ������� � ����� �� ��������
	// � ����� ������ �� ������� ��� ������ ��� ������ ������ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	// �� ���� ������� imageAvailableSemaphore
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores	  = waitSemaphores;
	submitInfo.pWaitDstStageMask  = waitStages;

	submitInfo.commandBufferCount = 1;
	// ������ ��������� ����� ������ � ���������� �����-�������
	// �� �� ����� ��������� ���� ����� ������ �����. �� ����������� ��� �����,
	// ������� ������ � ������ ������������ � ������� ������
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	// ��������� ����� �������� ������ ����� ���������� ������� ������ ������
	VkSemaphore signalSemaphores[]  = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores	= signalSemaphores;

	// ���������� ����� � ������������ ��������� ��� ����� �����
	// vkQueueSubmit ������� ��� ����� ���������� ����� �������� ���������� ������
	vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);
	// � ������� ����� ��������� ����� ��������� submitInfo, ������������������ �������
	// ��������� ��������� ���������� 
	// ��������� �������� ��� �����, ������� ����� ������� ���������� ����� ���������� 
	// ���� ������� ����� ���������
	if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer!");

	// ��� ��������� ���������� ��� ���� ����� ������� ����� � ����� ����������� �� ����� �������
	// ������� � ������� ������
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	// ������ ���������, ������� ���������� ��������� ������ ��� ����������� ����� ����� ����� �������� �� ������
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	// ����� ����������� ������� ������ � ������ �����������, ������� �� ���������� � ������� ��� ������ �� ���
	// ������ ������� ������ ���� ����
	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	// ������������ �������� ������� ��������� �������� ������ VkResult ��������, � ������� ����� ������� 
	// ��������� �� ���������� ������ ����������� ��� ������ ������� ������
	// ���� �������� ��������� ������ ���� �� ���������� ��������� ������� ������
	// ���� ������� ����, �� ���������� ���� ��� vkQueuePresentKHR ����� ���������� VkResult ��� ����� �� �������
	// �������� �����������
	presentInfo.pResults = nullptr; 

	result = vkQueuePresentKHR(presentationQueue, &presentInfo);

	// ���� � ������ ����� �������� � ������� ����������� ����������� ��� ������ �������� ������,
	// �� vkQueuePresentKHR �� ������ �������� ����������� ��� ��� ������� ������ �����
	// �� ���������� � �����������
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if(result != VK_SUCCESS)
		throw std::runtime_error("failed to present swap chain image!");

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::cleanupSwapChain()
{
	for(auto framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

	// ������ ���� ����� ��������� ����� ��������� ��� � �������� ������ � ���
	// �� ����� ���������� ������� � �������� ����� ��������� ������ � ���
	vkFreeCommandBuffers(logicalDevice, 
						 commandPool, 
						 static_cast<uint32_t>(commandBuffers.size()), 
						 commandBuffers.data());

	vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);

	for(auto imageView : swapChainImageViews)
		vkDestroyImageView(logicalDevice, imageView, nullptr);

	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);

	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		vkDestroyBuffer(logicalDevice, uniformBuffers[i], nullptr);
		vkFreeMemory(logicalDevice, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
}

void Renderer::cleanup()
{
	cleanupSwapChain();

	vkDestroyImage(logicalDevice, textureImage, nullptr);
	vkFreeMemory(logicalDevice, textureImageMemory, nullptr);

	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);

	vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
	vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);

	vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
	vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

	vkDestroyDevice(logicalDevice, nullptr);

	if(enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(pWindow);

	glfwTerminate();
}

