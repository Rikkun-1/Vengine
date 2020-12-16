#include "Renderer.h"

#include <chrono>
    
///////////////////////// STATIC BEG //////////////////////////////

static bool endsWith(std::string str1, std::string str2)
{
    size_t len1 = str1.length();
    size_t len2 = str2.length();

    if(len1 < len2) return false;

    std::string str1end = str1.substr(len1 - len2, len2);

    return str1end.compare(str2) == 0;
}

static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    Renderer *app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

static void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Renderer *app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
    if(app->interfaceCallback)
        app->interfaceCallback(key, action, mods, app);
}

static void drop_callback(GLFWwindow* window, int count, const char** paths)
{
    Renderer *app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
    std::string path = paths[0];

    if(endsWith(path, ".obj"))
    {
        Mesh mesh(path);
        app->setMesh(mesh);
        app->pushMesh();
    } 
    else if(endsWith(path, ".png") ||
            endsWith(path, ".jpg") ||
            endsWith(path, ".bmp"))
    {
        Texture texture(path);
        app->setTexture(texture);
        app->pushTexture();
    }
}

///////////////////////// STATIC END //////////////////////////////


///////////////////////// RENDERER BEG //////////////////////////////

Renderer::Renderer() 
{
}

void Renderer::loadShader(Shader shader) 
{
    if(shader.stage == ShaderStages::VERTEX_STAGE)
    {
        this->vertexShader = shader;
    } 
    else if(shader.stage == ShaderStages::FRAGMENT_STAGE)
    {
        this->fragmentShader = shader;
    }
}

void Renderer::setFillMode(FillMode fillMode)
{
    if (fillMode == FillMode::FILL)
        pipelineFixedFunctions.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    if (fillMode == FillMode::LINE)
        pipelineFixedFunctions.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;

    if (fillMode == FillMode::POINT)
        pipelineFixedFunctions.rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
    
    vkDeviceWaitIdle(device.handle);

    setupPipeline();

    writeCommandsForDrawing();
}

void Renderer::setModel(Model model) 
{
    this->model = model;
}

void Renderer::setMesh(Mesh mesh) 
{
    this->model.mesh = mesh;
}

void Renderer::setTexture(Texture texture) 
{
    this->model.texture = texture;
}

void Renderer::setInterfaceCallback(void (*interfaceCallbackFun)(int, int, int, Renderer *))
{
    this->interfaceCallback = interfaceCallbackFun;
}


void Renderer::pushModel() 
{
    pushMesh(false);
    pushTexture();
}

void Renderer::pushMesh(bool rewriteCommandBuffers) 
{      
    if(model.mesh.vertices.empty())
    {
        const std::vector<Vertex> vertices = {
            {{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
            {{1.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
        };

        model.mesh.vertices = vertices;

        std::vector<uint32_t> indices = {2, 3, 0, 0, 3, 1};
        model.mesh.indices = indices;
    }

    vkDeviceWaitIdle(device.handle);

    if(!vertexBuffer.device)
        vertexBuffer.setDevice(&device);

    if(!indexBuffer.device)
        indexBuffer.setDevice(&device);

    vertexBuffer.destroy();
    indexBuffer.destroy();

    createVertexBuffer(commandPool,
                       model.mesh.vertices,
                       vertexBuffer);
            
    createIndexBuffer(commandPool,
                      model.mesh.indices,
                      indexBuffer);

    if(rewriteCommandBuffers)
        writeCommandsForDrawing();
}

void Renderer::pushTexture(bool rewriteCommandBuffers) 
{
    if(model.texture.pixels.empty())
    {
        model.texture.pixels.push_back(Pixel{255, 255, 255, 255});
        model.texture.width    = 1;
        model.texture.height   = 1;
        model.texture.channels = 4;
    }

    VkExtent3D textureExtent{
        model.texture.getWidth(),
        model.texture.getHeight(),
        1
    };
    vkDeviceWaitIdle(device.handle);

    if(!textureImage.device)
        textureImage.setDevice(&device);

    vkDestroyImageView(device.handle, textureImageView, nullptr);
    textureImage.destroy();

    createTextureImage(model.texture.pixels.data(),
                       model.texture.getChannels(),
                       textureExtent,
                       commandPool,
                       textureImage);
    createTextureImageView(device, textureImage, textureImageView);

    if(descriptorSets.data())
        vkResetDescriptorPool(device.handle, descriptorPool, 0);

    createDescriptorSets(device,
                         descriptorPool,
                         descriptorSetLayout,
                         descriptorSets,
                         uniformBuffers,
                         textureImageView,
                         textureSampler,
                         swapChain.images.size());

    if(rewriteCommandBuffers)
        writeCommandsForDrawing();
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

    //pWindow = glfwCreateWindow(settings.windowWidth, settings.windowHeight, 
    //                          settings.windowTitle, glfwGetPrimaryMonitor(), nullptr);

    pWindow = glfwCreateWindow(settings.windowWidth, settings.windowHeight, 
                              settings.windowTitle, nullptr, nullptr);

    // ����������� � ���� ��������� �� ��������� ������ Renderer
    // ����� � framebufferResizeCallback ����� ������ � framebufferResized
    glfwSetWindowUserPointer(pWindow, this);
    glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);
    
    glfwSetKeyCallback(pWindow, glfwKeyCallback);
    glfwSetDropCallback(pWindow, drop_callback);
}

void Renderer::initVulkan()
{
    instance = createInstance(settings.validationLayers,
                              settings.instanceExtensions);

    debugMessenger = setupDebugMessenger(instance);
    
    if(glfwCreateWindowSurface(instance, pWindow, nullptr, &surface) != VK_SUCCESS)
        std::runtime_error("failed to create window surface");

    setupLogicalDevice();

    setupSwapchain();
    createUniformBuffers(device,
                         uniformBuffers,
                         swapChain.images.size());

    setupShaderModules();

    descriptorSetLayout = createDescriptorSetLayout(device.handle);
    pipelineLayout      = createPipelineLayout(device.handle, descriptorSetLayout);

    pipelineFixedFunctions.setup(swapChain.extent);
    setupPipeline();

    setupCommandPool();

    createDepthResources(commandPool,
                         swapChain.extent,
                         depthImage,
                         depthImageView);
    
    createTextureSampler(device, textureSampler);

    swapChain.createFrameBuffers(renderPass, depthImageView);
    
    descriptorPool = createDescriptorPool(device, swapChain.images.size());
    pushModel();

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
    int tick = 0;
    static float lastTime = 0;
    while(!glfwWindowShouldClose(pWindow))
    {
        glfwPollEvents();

        drawFrame();
    }
    
    // ������ ��� ����� �� �������� ����� �� ���� ����� ���������� ��������� ���������� ����������
    // ���������� ������ 
    vkDeviceWaitIdle(device.handle);
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
    vkWaitForFences(device.handle, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // ������, ����������� ����� ����������� � swapChainImages 
    // ����� ���������
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device.handle,
                                            swapChain.handle,
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
        vkWaitForFences(device.handle, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    
    // ��������� �����������, ���������� �� ������� ������ � �������, ������� ������
    // ���������� ����� ��������� � ��� ����������� ����� ��������
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    updateUniformBuffer(device.handle,
                        imageIndex,
                        swapChain.extent,
                        model.position,
                        model.rotation,
                        model.scale,
                        uniformBuffers);

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
    vkResetFences(device.handle, 1, &inFlightFences[currentFrame]);
    // � ������� ����� ��������� ����� ��������� submitInfo, ������������������ �������
    // ��������� ��������� ���������� 
    // ��������� �������� ��� �����, ������� ����� ������� ���������� ����� ���������� 
    // ���� ������� ����� ���������
    if(vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
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
    VkSwapchainKHR swapChains[] = {swapChain.handle};
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapChains;
    presentInfo.pImageIndices   = &imageIndex;

    // ������������ �������� ������� ��������� �������� ������ VkResult ��������, � ������� ����� ������� 
    // ��������� �� ���������� ������ ����������� ��� ������ ������� ������
    // ���� �������� ��������� ������ ���� �� ���������� ��������� ������� ������
    // ���� ������� ����, �� ���������� ���� ��� vkQueuePresentKHR ����� ���������� VkResult ��� ����� �� �������
    // �������� �����������
    presentInfo.pResults = nullptr; 

    result = vkQueuePresentKHR(device.presentQueue, &presentInfo);

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

void Renderer::setupShaderModules()
{
    if(vertexShaderModule.handle)
        vertexShaderModule.destroy();

    if(vertexShaderModule.handle)
        fragmentShaderModule.destroy();

    if(!vertexShaderModule.device)
        vertexShaderModule.setDevice(device);

    if(!fragmentShaderModule.device)
        fragmentShaderModule.setDevice(device);

    vertexShaderModule.create(vertexShader.binaryCode,
                              VK_SHADER_STAGE_VERTEX_BIT,
                              vertexShader.entry);

    fragmentShaderModule.create(fragmentShader.binaryCode,
                                VK_SHADER_STAGE_FRAGMENT_BIT,
                                fragmentShader.entry);
}

void Renderer::setupLogicalDevice()
{
    VkPhysicalDevice physicalDevice;
    physicalDevice = pickPhysicalDevice(instance, 
                                        surface, 
                                        settings.deviceExtensions);

    device = createLogicalDevice(instance,
                                 physicalDevice,
                                 surface, 
                                 settings.deviceExtensions);
}

void Renderer::setupSwapchain()
{    
    int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);

    VkExtent2D swapChainExtent {width, height};

    swapChain.create(device, surface, swapChainExtent);
}

void Renderer::setupCommandPool()
{    
    commandPool.setDevice(&this->device);
    commandPool.create();

    commandBuffers.resize(swapChain.images.size());

    commandPool.allocateCommandBuffers(commandBuffers.size(),
                                       commandBuffers.data());
}

void Renderer::setupPipeline()
{    
    if(!renderPass)
        renderPass = createRenderPass(device, swapChain.imageFormat);

    if(graphicsPipeline)
    {
        vkDestroyPipeline(device.handle, graphicsPipeline, nullptr);
        graphicsPipeline = VK_NULL_HANDLE;
    }

    graphicsPipeline = createGraphicsPipeline(device,
                                              swapChain.extent,
                                              pipelineFixedFunctions,
                                              renderPass,
                                              vertexShaderModule,
                                              fragmentShaderModule,
                                              descriptorSetLayout,
                                              pipelineLayout);
}

void Renderer::writeCommandsForDrawing()
{
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
}

void Renderer::recreateSwapChain()
{
    int width  = 0, 
        height = 0;
    glfwGetFramebufferSize(pWindow, &width, &height);

    // ���� ���� ��������, �� glfwGetFramebufferSize ������ ����
    // ������ ������ � ������. ������� �� ���� ���� ������ ���� �� ������ ���������� ��������
    // ��� ���������� ����� ���� ����� ����� ����������
    while(width == 0 || height == 0)
    {
        glfwGetFramebufferSize(pWindow, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device.handle);

    cleanupSwapChain();

    VkExtent2D swapChainExtent = {width, height};
    swapChain.create(device, surface, swapChainExtent);

    pipelineFixedFunctions.setupViewPortAndScissor(swapChain.extent);
    setupPipeline();

    createDepthResources(commandPool,
                         swapChain.extent,
                         depthImage,
                         depthImageView);

    swapChain.createFrameBuffers(renderPass, depthImageView);
    
    if(descriptorSets.data())
        vkResetDescriptorPool(device.handle, descriptorPool, 0);

    createDescriptorSets(device, 
                         descriptorPool,
                         descriptorSetLayout,
                         descriptorSets,
                         uniformBuffers,
                         textureImageView,
                         textureSampler,
                         swapChain.images.size());

    commandPool.allocateCommandBuffers(swapChain.images.size(),
                                       commandBuffers.data());
    writeCommandsForDrawing();
}

void Renderer::cleanupSwapChain()
{
    vkDestroyImageView(device.handle, depthImageView, nullptr);
    depthImage.destroy();

    // ������ ���� ����� ��������� ����� ��������� ��� � �������� ������ � ���
    // �� ����� ���������� ������� � �������� ����� ��������� ������ � ���

    vkResetCommandPool(device.handle, commandPool.handle, 0);

    swapChain.destroy();
}

void Renderer::cleanup()
{
    cleanupSwapChain();

    for(size_t i = 0; i < uniformBuffers.size(); i++)
        uniformBuffers[i].destroy();
    
    vkDestroyPipeline(device.handle, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device.handle, pipelineLayout, nullptr);
    vkDestroyRenderPass(device.handle, renderPass, nullptr);

    vkDestroyDescriptorPool(device.handle, descriptorPool, nullptr);

    vkDestroySampler(device.handle, textureSampler, nullptr);
    vkDestroyImageView(device.handle, textureImageView, nullptr);
    textureImage.destroy();

    vkDestroyDescriptorSetLayout(device.handle, descriptorSetLayout, nullptr);

    indexBuffer.destroy();
    vertexBuffer.destroy();

    vertexShaderModule.destroy();
    fragmentShaderModule.destroy();

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device.handle, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device.handle, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence    (device.handle, inFlightFences[i],           nullptr);
    }
    
    vkDestroyCommandPool(device.handle, commandPool.handle, nullptr);
    vkDestroyDevice(device.handle, nullptr);

    if(enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(pWindow);

    glfwTerminate();
}

///////////////////////// RENDERER END //////////////////////////////
