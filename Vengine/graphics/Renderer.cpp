#include "Renderer.h"

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

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

    // ����������� � ���� ��������� �� ��������� ������ Renderer
    // ����� � framebufferResizeCallback ����� ������ � framebufferResized
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

    VkExtent2D swapChainExtent {width, height};

    swapChain.create(device, surface, swapChainExtent);

    renderPass          = createRenderPass(device, swapChain.imageFormat);

    descriptorSetLayout = createDescriptorSetLayout(device.handle);

    pipelineLayout      = createPipelineLayout(device.handle, descriptorSetLayout);

    graphicsPipeline    = createGraphicsPipeline(device,
                                                 swapChain.extent,
                                                 renderPass,
                                                 vertexShader,
                                                 fragmentShader,
                                                 descriptorSetLayout,
                                                 pipelineLayout);

    commandPool.setDevice(&this->device);
    commandPool.create();

    createDepthResources(commandPool,
                         swapChain.extent,
                         depthImage,
                         depthImageView);

    swapChain.createFrameBuffers(renderPass, depthImageView);

    VkExtent3D textureExtent {
        model.texture.getWidth(),
        model.texture.getHeight(),
        1
    };

    createTextureImage(model.texture.getRaw(),
                       model.texture.getChannels(),
                       textureExtent,
                       commandPool,
                       textureImage);

    createTextureImageView(device, textureImage, textureImageView);

    createTextureSampler(device, textureSampler);

    createVertexBuffer(commandPool,
                       model.mesh.vertices,
                       vertexBuffer);

    createIndexBuffer(commandPool,
                      model.mesh.indices,
                      indexBuffer);

    createUniformBuffers(device,
                         uniformBuffers,
                         swapChain.images.size());

    createDescriptorPool(device, swapChain.images.size());

    createDescriptorSets(device, 
                         descriptorPool,
                         descriptorSetLayout,
                         descriptorSets,
                         uniformBuffers,
                         textureImageView,
                         textureSampler,
                         swapChain.images.size());

    commandPool.allocateCommandBuffers(static_cast<uint32_t>(model.mesh.indices.size()),
                                       commandBuffers.data());

    writeCommandBuffersForDrawing(commandPool,
                                  swapChain,
                                  renderPass,
                                  graphicsPipeline,
                                  pipelineLayout,
                                  vertexBuffer.handle,
                                  indexBuffer.handle,
                                  model.mesh.indices.size(),
                                  descriptorSets,
                                  commandBuffers);

    createSyncObjects(device,
                      MAX_FRAMES_IN_FLIGHT,
                      swapChain,
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
    vkDeviceWaitIdle(device.handle);
}

void Renderer::recreateSwapChain()
{

    
    int width  = 0, 
        height = 0;
    glfwGetFramebufferSize(pWindow, &width, &height);

    // ���� ���� ��������, �� glfwGetFramebufferSize ������ ����
    // ������ ������ � ������. ������� �� ���� ���� ������ ���� �� ������ ���������� ��������
    // ��� ���������� ����� ���� ����� ����� ����������
    std::cout << width << "  " << height << std::endl;
    while(width == 0 || height == 0)
    {
        std::cout << width << " || " << height << std::endl;
        glfwGetFramebufferSize(pWindow, &width, &height);
        glfwWaitEvents();
    }
    

    vkDeviceWaitIdle(device.handle);

    cleanupSwapChain();

    VkExtent2D swapChainExtent = {width, height};
    swapChain.destroy();
    swapChain.create(device, surface, swapChainExtent);

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
    submitInfo.waitSemaphoreCount     = 1;
    submitInfo.pWaitSemaphores        = waitSemaphores;
    submitInfo.pWaitDstStageMask      = waitStages;

    submitInfo.commandBufferCount = 1;
    // ������ ��������� ����� ������ � ���������� �����-�������
    // �� �� ����� ��������� ���� ����� ������ �����. �� ����������� ��� �����,
    // ������� ������ � ������ ������������ � ������� ������
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    // ��������� ����� �������� ������ ����� ���������� ������� ������ ������
    VkSemaphore signalSemaphores[]  = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

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
    presentInfo.pWaitSemaphores    = signalSemaphores;

    // ����� ����������� ������� ������ � ������ �����������, ������� �� ���������� � ������� ��� ������ �� ���
    // ������ ������� ������ ���� ����
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapChains;
    presentInfo.pImageIndices   = &imageIndex;

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
    vkDestroyImageView(device.handle, depthImageView, nullptr);

    depthImage.destroy();

    // ������ ���� ����� ��������� ����� ��������� ��� � �������� ������ � ���
    // �� ����� ���������� ������� � �������� ����� ��������� ������ � ���

    commandPool.freeCommandBuffers(commandBuffers.size(),
                                   commandBuffers.data());

    vkDestroyPipeline(device.handle, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device.handle, pipelineLayout, nullptr);
    vkDestroyRenderPass(device.handle, renderPass, nullptr);

    for(size_t i = 0; i < swapChain.images.size(); i++)
        uniformBuffers[i].destroy();

    swapChain.destroy();

    vkDestroyDescriptorPool(device.handle, descriptorPool, nullptr);
}

void Renderer::cleanup()
{
    cleanupSwapChain();

    vkDestroySampler(device.handle, textureSampler, nullptr);
    vkDestroyImageView(device.handle, textureImageView, nullptr);

    textureImage.destroy();

    vkDestroyDescriptorSetLayout(device.handle, descriptorSetLayout, nullptr);

    indexBuffer.destroy();
    vertexBuffer.destroy();

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device.handle, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device.handle, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence    (device.handle, inFlightFences[i],           nullptr);
    }

    vkDestroyDevice(device.handle, nullptr);

    if(enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(pWindow);

    glfwTerminate();
}

