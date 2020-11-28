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

	// прежде чем выйти из главного цикла мы ждем чтобы видеокарта закончила выполнения последнего
	// командного буфера 
	vkDeviceWaitIdle(logicalDevice);
}

void Renderer::drawFrame()
{
	// ожидаем чтобы все заборы перешли в сигнальное состояние
	// функия принимает массив заборов и в зависимости от 4-го параметра 
	// ждет либо один любой забор, либо сразу все
	// последний параметр это то как долго мы собираемся ждать в наносекундах
	// функция завершится не дожидаясь заборов если время выйдет. В этом случае она вернет VK_TIMEOUT
	// иначе VK_SUCCESS
	// значение UINT64_MAX означает что ограничения по времени нет
	vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	// переводим заборы в несигнальное состояние
	vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

	// индекс, указывающий какое изображение в swapChainImages 
	// стало доступным
	uint32_t imageIndex;
	vkAcquireNextImageKHR(logicalDevice,
						  swapChain, 
						  // время в наносекундах, данное изображению
						  // чтобы стать доступным. значение UINT64_MAX - отключает счетчик
						  UINT64_MAX,
						  // семафор, который нужно сделать сигнальным когда изображение будет готово для использования
						  imageAvailableSemaphores[currentFrame],
						  // аналогично, забор. Можно использовать оба одновременно. 
						  // Мы же используем только семафор
						  VK_NULL_HANDLE, 
						  // в imageIndex будет записан индекс изображения, которое
						  // стало доступным
						  &imageIndex); 

	// если предыдущий кадр все еще использует это изображение, то мы ждем до тех пор пока 
	// оно не освободится
	if(imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

	// связываем изображение, полученное из цепочки показа с забором, который станет
	// сигнальным когда рисование в это изображение будет окончено
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// описываем семафоры, котоые необходимо подождать до начала
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};

	// описываем во время какой стадии мы ожидаем семафоры
	// каждый семафор соотносится со стадией с таким же индексом
	// в нашем случае мы говорим что прежде чем начать стадию VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	// мы ждем семафор imageAvailableSemaphore
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores	  = waitSemaphores;
	submitInfo.pWaitDstStageMask  = waitStages;

	submitInfo.commandBufferCount = 1;
	// каждый командный буфер связан с конкретным фрейм-буфером
	// мы не можем привязать сюда какой попало буфер. Мы привязываем тот буфер,
	// который связан с данным изображением в цепочке показа
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	// описывает какие семафоры зажечь после исполнения данного буфера команд
	VkSemaphore signalSemaphores[]  = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores	= signalSemaphores;

	// сбрасываем забор в несигнальное состояние для этого кадра
	// vkQueueSubmit сделает его вновь сигнальным когда закончит выполнение команд

	vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);
	// в очерень можно отправить сразу несколько submitInfo, последовательность которых
	// настроена благодаря светофорам 
	// последний параметр это забор, который нужно сделать сигнальным когда выполнение 
	// всех буферов будет закончено
	if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer!");

	// эта структура необходима для того чтобы описать когда и какое изображение мы хотим вернуть
	// обратно в цепочку показа
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	// список семафоров, которые необходимо дождаться прежде чем изображение можно будет вновь рисовать на экране
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	// здесь описываются цепочки показа и индекс изображения, которое мы возвращаем в цепочку для каждой из них
	// обычно цепочка показа лишь одна
	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	// опциональный параметр который позволяет передать массив VkResult значений, в который будет записан 
	// результат об успешности показа изображения для каждой цепочки показа
	// Этот параметр требуется только если мы используем несколько цепочек показа
	// если цепочка одна, то достаточно того что vkQueuePresentKHR также возвращает VkResult как ответ на попытку
	// показать изображение
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
