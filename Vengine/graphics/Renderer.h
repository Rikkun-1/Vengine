#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkanWrapper/vulkanWrapper.h"
#include "settings.h"
#include "Model.h"
#include "Shader.h"

class Renderer
{
public:
    bool framebufferResized = false;

    vkSettings settings;
    PipelineFixedFunctions pipelineFixedFunctions;

    Model   model;

    Renderer();

    void run();

    void setModel(Model model);
    void setMesh(Mesh mesh);
    void setTexture(Texture texture);
    void setInterfaceCallback(void (*interfaceCallback)(int, int, int, Renderer *));

    void pushModel();
    void pushMesh   (bool rewriteCommandBuffers = true);
    void pushTexture(bool rewriteCommandBuffers = true);

    void loadShader(Shader shader);
    
    void setupPipeline();

private:
    void (*interfaceCallback)(int, int, int, Renderer *);

    VkInstance               instance;

    VkDebugUtilsMessengerEXT debugMessenger;

    GLFWwindow               *pWindow;
    VkSurfaceKHR             surface;

    LogicalDevice            device;
    Swapchain                swapChain;
    
    VkRenderPass               renderPass;
    VkDescriptorSetLayout      descriptorSetLayout;
    VkPipelineLayout           pipelineLayout;

    VkPipeline                 graphicsPipeline;

    CommandPool                   commandPool;
    std::vector<VkCommandBuffer>  commandBuffers;

    std::vector<VkSemaphore>    imageAvailableSemaphores;
    std::vector<VkSemaphore>    renderFinishedSemaphores;
    std::vector<VkFence>        inFlightFences;
    std::vector<VkFence>        imagesInFlight;

    Buffer                      vertexBuffer;
    Buffer                      indexBuffer;
    std::vector<Buffer>         uniformBuffers;
        
    Image                        textureImage;
    VkImageView                  textureImageView;
    VkSampler                    textureSampler;

    Image                        depthImage;
    VkImageView                  depthImageView;

    VkDescriptorPool             descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;


    int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame      = 0;

    Shader vertexShader;
    Shader fragmentShader;

    ShaderModule vertexShaderModule;
    ShaderModule fragmentShaderModule;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void drawFrame();
    void setupShaderModules();
    void setupLogicalDevice();
    void setupSwapchain();
    void setupCommandPool();
    void writeCommandsForDrawing();
    void recreateSwapChain();
    void cleanupSwapChain();
    void cleanup();

    friend void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
};

