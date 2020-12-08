#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkanWrapper/vulkanWrapper.h"
#include "vkSettings.h"
#include "vulkanWrapper/Vertex.h"

class Renderer
{
public:
    vkSettings settings;
    bool framebufferResized = false;

    void run();

private:
    GLFWwindow               *pWindow;
    VkInstance               instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR             surface;

    VkPhysicalDevice         physicalDevice;
    VkDevice                 logicalDevice;

    VkQueue                  graphicsQueue;
    VkQueue                  presentationQueue;

    VkSwapchainKHR           swapChain;
    std::vector<VkImage>     swapChainImages;
    VkFormat                 swapChainImageFormat;
    VkExtent2D               swapChainExtent;

    std::vector<VkImageView>   swapChainImageViews;

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

    VkDescriptorPool             descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout      pipelineLayout;

    int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame      = 0;

    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    void initWindow();
    void initVulkan();
    void mainLoop();
    void drawFrame();
    void recreateSwapChain();
    void cleanupSwapChain();
    void cleanup();
};

