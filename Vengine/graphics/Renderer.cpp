#include "Renderer.h"

static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void Renderer::loadShader(Shader shader) 
{
    if(shader.stage == ShaderStages::VERTEX_STAGE)
    {
        ShaderModule shaderModule(device.handle,
                                  shader.binaryCode,
                                  VK_SHADER_STAGE_VERTEX_BIT,
                                  shader.entry);

        this->vertexShader = shaderModule;
    }

    if(shader.stage == ShaderStages::VERTEX_STAGE)
    {
         ShaderModule shaderModule(device.handle,
                                  shader.binaryCode,
                                  VK_SHADER_STAGE_FRAGMENT_BIT,
                                  shader.entry);

        this->fragmentShader = shaderModule;
    }
}

void Renderer::changeModel(Model model) 
{
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

    // прикрепляем к окну указатель на экземпляр класса Renderer
    // чтобы в framebufferResizeCallback иметь доступ к framebufferResized
    glfwSetWindowUserPointer(pWindow, this);
    glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);
}

void Renderer::initVulkan()
{
    instance = createInstance(settings.validationLayers,
                              settings.instanceExtensions);

    debugMessenger = setupDebugMessenger(instance);

    if(glfwCreateWindowSurface(instance, pWindow, nullptr, &surface) != VK_SUCCESS)
        std::runtime_error("failed to create window surface");

    VkPhysicalDevice physicalDevice;
    physicalDevice = pickPhysicalDevice(instance, 
                                        surface, 
                                        settings.deviceExtensions);

    device = createLogicalDevice(instance,
                                 physicalDevice,
                                 surface, 
                                 settings.deviceExtensions);

    int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);

    VkExtent2D extent {width, height};

    createSwapChain(device,
                    surface,
                    extent,
                    this->swapChain);

    this->renderPass          = createRenderPass(device, swapChain.imageFormat);

    this->descriptorSetLayout = createDescriptorSetLayout(device.handle);

    this->pipelineLayout      = createPipelineLayout(device.handle, descriptorSetLayout);

    createGraphicsPipeline(logicalDevice,
                           swapChainExtent,
                           renderPass,
                           pipelineLayout,
                           graphicsPipeline,
                           descriptorSetLayout);

    createCommandPool(physicalDevice,
                      logicalDevice,
                      surface,
                      commandPool);

    createDepthResources(physicalDevice,
                         logicalDevice,
                         commandPool,
                         graphicsQueue,
                         swapChainExtent,
                         depthImage,
                         depthImageMemory,
                         depthImageView);

    createFramebuffers(logicalDevice,
                       renderPass,
                       swapChainExtent,
                       swapChainImageViews,
                       swapChainFramebuffers,
                       depthImageView);

    createTextureImage(physicalDevice,
                       logicalDevice,
                       commandPool,
                       graphicsQueue,
                       textureImage,
                       textureImageMemory);

    createTextureImageView(logicalDevice,
                           textureImage,
                           textureImageView);

    createTextureSampler(physicalDevice,
                         logicalDevice,
                         textureSampler);

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
                         uniformBuffers,
                         textureImageView,
                         textureSampler);

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

    // прежде чем выйти из главного цикла мы ждем чтобы видеокарта закончила выполнения последнего
    // командного буфера 
    vkDeviceWaitIdle(logicalDevice);
}

void Renderer::recreateSwapChain()
{

    
    int width  = 0, 
        height = 0;
    glfwGetFramebufferSize(pWindow, &width, &height);

    // если окно свернуто, то glfwGetFramebufferSize вернет нули
    // вместо ширины и высоты. Поэтому мы ждем пока размер окна не примет адекватные значения
    // это произойдет когда окно вновь будет развернуто
    std::cout << width << "  " << height << std::endl;
    while(width == 0 || height == 0)
    {
        std::cout << width << " || " << height << std::endl;
        glfwGetFramebufferSize(pWindow, &width, &height);
        glfwWaitEvents();
    }
    

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

    createRenderPass(physicalDevice,
                     logicalDevice,
                     swapChainImageFormat,
                     renderPass);

    createGraphicsPipeline(logicalDevice,
                           swapChainExtent,
                           renderPass,
                           pipelineLayout,
                           graphicsPipeline,
                           descriptorSetLayout);

    createDepthResources(physicalDevice,
                         logicalDevice,
                         commandPool,
                         graphicsQueue,
                         swapChainExtent,
                         depthImage,
                         depthImageMemory,
                         depthImageView);

    createFramebuffers(logicalDevice,
                       renderPass,
                       swapChainExtent,
                       swapChainImageViews,
                       swapChainFramebuffers,
                       depthImageView);

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
                         uniformBuffers,
                         textureImageView,
                         textureSampler);

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
    // ожидаем чтобы все заборы перешли в сигнальное состояние
    // функия принимает массив заборов и в зависимости от 4-го параметра 
    // ждет либо один любой забор, либо сразу все
    // последний параметр это то как долго мы собираемся ждать в наносекундах
    // функция завершится не дожидаясь заборов если время выйдет. В этом случае она вернет VK_TIMEOUT
    // иначе VK_SUCCESS
    // значение UINT64_MAX означает что ограничения по времени нет
    vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // индекс, указывающий какое изображение в swapChainImages 
    // стало доступным
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(logicalDevice,
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
    
    // если vkAcquireNextImageKHR вернул VK_ERROR_OUT_OF_DATE_KHR
    // то это значит что текущий формат изображений в цепочке показа более не соответсвует 
    // поверхности к которой привязана цепочка показа
    // такое может произойти например при изменении размера окна
    // VK_SUBOPTIMAL_KHR означает что изображения все еще могут использоваться для отрисовки,
    // но они более не являются изображениями, полностью совпадающими с поверхностью на экране
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        // мы пересоздаем цепочку показа чтобы она вновь стала совместимой с поверхностью для показа
        recreateSwapChain();
        // мы не получили изображение для отрисовки текущего кадра. 
        // без изображения дальнейший код не может исполняться
        return;
    }
    else if(result != VK_SUCCESS)
        throw std::runtime_error("failed to acquire swap chain image!");

    // если предыдущий кадр все еще использует это изображение, то мы ждем до тех пор пока 
    // оно не освободится
    if(imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    
    // связываем изображение, полученное из цепочки показа с забором, который станет
    // сигнальным когда рисование в это изображение будет окончено
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    updateUniformBuffer(logicalDevice,
                        imageIndex,
                        swapChainExtent,
                        uniformBuffersMemory);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // описываем семафоры, котоые необходимо подождать до начала
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};

    // описываем во время какой стадии мы ожидаем семафоры
    // каждый семафор соотносится со стадией с таким же индексом
    // в нашем случае мы говорим что прежде чем начать стадию VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    // мы ждем семафор imageAvailableSemaphore
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount     = 1;
    submitInfo.pWaitSemaphores        = waitSemaphores;
    submitInfo.pWaitDstStageMask      = waitStages;

    submitInfo.commandBufferCount = 1;
    // каждый командный буфер связан с конкретным фрейм-буфером
    // мы не можем привязать сюда какой попало буфер. Мы привязываем тот буфер,
    // который связан с данным изображением в цепочке показа
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    // описывает какие семафоры зажечь после исполнения данного буфера команд
    VkSemaphore signalSemaphores[]  = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

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
    presentInfo.pWaitSemaphores    = signalSemaphores;

    // здесь описываются цепочки показа и индекс изображения, которое мы возвращаем в цепочку для каждой из них
    // обычно цепочка показа лишь одна
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapChains;
    presentInfo.pImageIndices   = &imageIndex;

    // опциональный параметр который позволяет передать массив VkResult значений, в который будет записан 
    // результат об успешности показа изображения для каждой цепочки показа
    // Этот параметр требуется только если мы используем несколько цепочек показа
    // если цепочка одна, то достаточно того что vkQueuePresentKHR также возвращает VkResult как ответ на попытку
    // показать изображение
    presentInfo.pResults = nullptr; 

    result = vkQueuePresentKHR(presentationQueue, &presentInfo);

    // если в момент между рендером и показом изображения поверхность для показа изменила размер,
    // то vkQueuePresentKHR не сможет показать изображение так как цепочка показа более
    // не совместима с повехностью
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
    vkDestroyImageView(logicalDevice, depthImageView, nullptr);
    vkDestroyImage(logicalDevice, depthImage, nullptr);
    vkFreeMemory(logicalDevice, depthImageMemory, nullptr);

    for(auto framebuffer : swapChainFramebuffers)
        vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

    // вместо того чтобы создавать новый командный пул и выделять буферы в нем
    // мы можем освободить текущий и выделить новые командные буферы в нем
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

    vkDestroySampler(logicalDevice, textureSampler, nullptr);
    vkDestroyImageView(logicalDevice, textureImageView, nullptr);

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


    vkDestroyDevice(logicalDevice, nullptr);

    if(enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(pWindow);

    glfwTerminate();
}

