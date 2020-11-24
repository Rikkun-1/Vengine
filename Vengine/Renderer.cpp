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

	createSemaphores();
}

void Renderer::mainLoop()
{
	while(!glfwWindowShouldClose(pWindow))
	{
		glfwPollEvents();
		drawFrame();
	}
}

void Renderer::drawFrame()
{
	// индекс, указывающий какое изображение в swapChainImages 
	// стало доступным
	uint32_t imageIndex;
	vkAcquireNextImageKHR(logicalDevice,
						  swapChain, 
						  // врем€ в наносекундах, данное изображению
						  // чтобы стать доступным. значение UINT64_MAX - отключает счетчик
						  UINT64_MAX,
						  // семафор, который нужно сделать сигнальным когда изображение будет готово дл€ использовани€
						  imageAvailableSemaphore, 
						  // аналогично, забор. ћожно использовать оба одновременно. 
						  // ћы же используем только семафор
						  VK_NULL_HANDLE, 
						  // в imageIndex будет записан индекс изображени€, которое
						  // стало доступным
						  &imageIndex); 

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// описываем семафоры, котоые необходимо подождать до начала
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};

	// описываем во врем€ какой стадии мы ожидаем семафоры
	// каждый семафор соотноситс€ со стадией с таким же индексом
	// в нашем случае мы говорим что прежде чем начать стадию VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	// мы ждем семафор imageAvailableSemaphore
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores	  = waitSemaphores;
	submitInfo.pWaitDstStageMask  = waitStages;

	submitInfo.commandBufferCount = 1;
	// каждый командный буфер св€зан с конкретным фрейм-буфером
	// мы не можем прив€зать сюда какой попало буфер. ћы прив€зываем тот буфер,
	// который св€зан с данным изображением в цепочке показа
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	// описывает какие семафоры зажечь после исполнени€ данного буфера команд
	VkSemaphore signalSemaphores[]  = {renderFinishedSemaphore};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores	= signalSemaphores;

	// в очерень можно отправить сразу несколько submitInfo, последовательность которых
	// настроена благодар€ светофорам 
	// последний параметр это забор, который нужно сделать сигнальным когда выполнение 
	// всех буферов будет закончено
	if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer!");
	
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(presentationQueue, &presentInfo);
}

void Renderer::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	// согласно текущему состо€нию Vulkan API дл€ создани€ семафора более не требуетс€ никакой
	// информации кроме sType
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
	   vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
	{

		throw std::runtime_error("failed to create semaphores!");
	}
}

void Renderer::cleanup()
{
	vkDestroySemaphore(logicalDevice, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, nullptr);

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
