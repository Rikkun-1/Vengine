#include "Renderer.h"

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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	pWindow = glfwCreateWindow(settings.windowWidth, settings.windowHeight, 
							  settings.windowTitle, nullptr, nullptr);
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
	

	createGraphicsPipeline(logicalDevice,
						   swapChainExtent,
						   renderPass,
						   pipelineLayout,
						   graphicsPipeline);


	createFramebuffers(logicalDevice,
					   renderPass,
					   swapChainExtent,
					   swapChainImageViews,
					   swapChainFramebuffers);


	createCommandPool(physicalDevice,
					  logicalDevice,
					  surface,
					  commandPool);


	createCommandBuffers(logicalDevice,
						 swapChainExtent,
						 swapChainFramebuffers,
						 graphicsPipeline,
						 renderPass,
						 commandPool,
						 commandBuffers);

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
	// ��������� ������ � ������������ ���������
	vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

	// ������, ����������� ����� ����������� � swapChainImages 
	// ����� ���������
	uint32_t imageIndex;
	vkAcquireNextImageKHR(logicalDevice,
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

	// ���� ���������� ���� ��� ��� ���������� ��� �����������, �� �� ���� �� ��� ��� ���� 
	// ��� �� �����������
	if(imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

	// ��������� �����������, ���������� �� ������� ������ � �������, ������� ������
	// ���������� ����� ��������� � ��� ����������� ����� ��������
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

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

	vkQueuePresentKHR(presentationQueue, &presentInfo);
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::cleanup()
{
	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

	for(auto framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

	vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);

	for(auto imageView : swapChainImageViews)
		vkDestroyImageView(logicalDevice, imageView, nullptr);

	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);

	vkDestroyDevice(logicalDevice, nullptr);

	if(enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(pWindow);

	glfwTerminate();
}
