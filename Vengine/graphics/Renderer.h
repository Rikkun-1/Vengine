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

    void run();

    void changeModel(Model model);

    void loadShader(Shader shader);

private:
    VkInstance               instance;

    VkDebugUtilsMessengerEXT debugMessenger;

    GLFWwindow               *pWindow;
    VkSurfaceKHR             surface;

    LogicalDevice            device;
    SwapChain                swapChain;

    VkRenderPass               renderPass;

    VkPipeline                 graphicsPipeline;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkCommandPool                 commandPool;
    std::vector<VkCommandBuffer>  commandBuffers;

    std::vector<VkSemaphore>    imageAvailableSemaphores;
    std::vector<VkSemaphore>    renderFinishedSemaphores;
    std::vector<VkFence>        inFlightFences;
    std::vector<VkFence>        imagesInFlight;

    VkBuffer                    vertexBuffer;
    VkDeviceMemory              vertexBufferMemory;
    VkBuffer                    indexBuffer;
    VkDeviceMemory              indexBufferMemory;
    std::vector<VkBuffer>       uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory; 
        
    VkImage        textureImage;
    VkImageView    textureImageView;
    VkDeviceMemory textureImageMemory;
    VkSampler      textureSampler;

    VkImage        depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView    depthImageView;

    VkDescriptorPool             descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout      pipelineLayout;

    int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame      = 0;

    Model model;

    ShaderModule vertexShader;
    ShaderModule fragmentShader;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void drawFrame();
    void recreateSwapChain();
    void cleanupSwapChain();
    void cleanup();
};
