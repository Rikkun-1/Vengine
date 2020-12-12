#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkanWrapper/vulkanWrapper.h"
#include "vkSettings.h"
#include "Model.h"
#include "Shader.h"

class Renderer
{
public:
    vkSettings settings;
    bool framebufferResized = false;

    Renderer();

    void run();

    void setModel(Model model);
    void setMesh(Mesh mesh);
    void setTexture(Texture texture);
    void pushModel();
    void pushMesh   (bool rewriteCommandBuffers = true);
    void pushTexture(bool rewriteCommandBuffers = true);

    void loadShader(Shader shader);

private:
    VkInstance               instance;

    VkDebugUtilsMessengerEXT debugMessenger;

    GLFWwindow               *pWindow;
    VkSurfaceKHR             surface;

    LogicalDevice            device;
    SwapChain                swapChain;
    
    VkRenderPass               renderPass;
    VkDescriptorSetLayout      descriptorSetLayout;
    VkPipelineLayout           pipelineLayout;

    VkPipeline                 graphicsPipeline;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    CommandPool                   commandPool;
    std::vector<VkCommandBuffer>  commandBuffers;

    std::vector<VkSemaphore>    imageAvailableSemaphores;
    std::vector<VkSemaphore>    renderFinishedSemaphores;
    std::vector<VkFence>        inFlightFences;
    std::vector<VkFence>        imagesInFlight;

    Buffer                      vertexBuffer;
    Buffer                      indexBuffer;
    std::vector<Buffer>         uniformBuffers;
        
    Image                       textureImage;
    VkImageView                 textureImageView;
    VkSampler                   textureSampler;

    Image                       depthImage;
    VkImageView                 depthImageView;

    VkDescriptorPool             descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;


    int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame      = 0;

    Model   model;

    Shader vertexShader;
    Shader fragmentShader;

    ShaderModule vertexShaderModule;
    ShaderModule fragmentShaderModule;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void drawFrame();
    void writeCommandsForDrawing();
    void recreateSwapChain();
    void cleanupSwapChain();
    void cleanup();
};

