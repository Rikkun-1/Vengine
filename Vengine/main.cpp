#include <iostream>
#include <stdexcept>
#include <cstdlib> // ������� EXIT_SUCCESS � EXIT_FAILURE

#ifdef NDEBUG
    #define USE_VALIDATION_LAYERS
#endif

#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "graphics/Model.h"
#include "graphics/Renderer.h"
#include "graphics/Shader.h"

#include "graphics/tools.h"

#include <chrono>


void interfaceCallBack(int key, int action, int modificators, Renderer *renderer)
{
    static float onePressPosition = 0.05;
    static float onePressRotation = 2.5;
    static float onePressScale    = 0.05;

    glm::vec3 &position = renderer->model.position;
    glm::vec3 &rotation = renderer->model.rotation;
    glm::vec3 &scale    = renderer->model.scale;
    PipelineFixedFunctions &fixedFunctions = renderer->pipelineFixedFunctions;
    
    if(modificators == 0)
    {
        if(key == GLFW_KEY_KP_4) position.x += onePressPosition;
        if(key == GLFW_KEY_KP_6) position.x -= onePressPosition;
        if(key == GLFW_KEY_KP_8) position.y -= onePressPosition;
        if(key == GLFW_KEY_KP_2) position.y += onePressPosition;
        if(key == GLFW_KEY_KP_9) position.z += onePressPosition;
        if(key == GLFW_KEY_KP_3) position.z -= onePressPosition;
    }

    if(modificators == GLFW_MOD_SHIFT)
    {
        if(key == GLFW_KEY_KP_8) rotation.x += onePressRotation;
        if(key == GLFW_KEY_KP_2) rotation.x -= onePressRotation;
        if(key == GLFW_KEY_KP_9) rotation.y -= onePressRotation;
        if(key == GLFW_KEY_KP_3) rotation.y += onePressRotation;
        if(key == GLFW_KEY_KP_4) rotation.z -= onePressRotation;
        if(key == GLFW_KEY_KP_6) rotation.z += onePressRotation;
    }

    if(modificators == GLFW_MOD_ALT)
    {
        if(key == GLFW_KEY_KP_4) scale.x += onePressScale;
        if(key == GLFW_KEY_KP_6) scale.x -= onePressScale;
        if(key == GLFW_KEY_KP_8) scale.y -= onePressScale;
        if(key == GLFW_KEY_KP_2) scale.y += onePressScale;
        if(key == GLFW_KEY_KP_9) scale.z += onePressScale;
        if(key == GLFW_KEY_KP_3) scale.z -= onePressScale;
    }

    if(key == GLFW_KEY_F) fixedFunctions.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    if(key == GLFW_KEY_L) fixedFunctions.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
    if(key == GLFW_KEY_P) fixedFunctions.rasterizer.polygonMode = VK_POLYGON_MODE_POINT;

    if(key == GLFW_KEY_F || key == GLFW_KEY_L || key == GLFW_KEY_P)
        renderer->setupPipeline();

    Pixel color;
    if(key == GLFW_KEY_1) color = Pixel{204, 255,   0}; // ���������
    if(key == GLFW_KEY_2) color = Pixel{228,   0, 225}; // �������
    if(key == GLFW_KEY_3) color = Pixel{ 65, 179, 247}; //
    if(key == GLFW_KEY_4) color = Pixel{222,  93, 148}; //
    if(key == GLFW_KEY_5) color = Pixel{255, 172,  78}; // 
    if(key == GLFW_KEY_6) color = Pixel{222,  93,  96}; // 
    if(key == GLFW_KEY_7) color = Pixel{255,   0,   0}; // 
    if(key == GLFW_KEY_8) color = Pixel{  0, 255,   0}; // 
    if(key == GLFW_KEY_9) color = Pixel{  0,   0, 255}; // 
    if(key == GLFW_KEY_0) color = Pixel{255, 255, 255}; // 

    if(key >= 48 && key <= 57)
    {
        Texture justColor;
        justColor.pixels.push_back(color);
        justColor.width    = 1;
        justColor.height   = 1;
        justColor.channels = 4;

        renderer->setTexture(justColor);
        renderer->pushTexture();
    }
    
    if(key == GLFW_KEY_T)
    {
        std::string path;
        system("ECHO \"Enter texture path\" & ");
    }
    if(key == GLFW_KEY_M) color = Pixel{255, 255, 255}; // 
}


int main() 
{


    Shader vertexShader("shaders/bin/vert.spv", ShaderStages::VERTEX_STAGE);
    Shader fragmentShader("shaders/bin/frag.spv", ShaderStages::FRAGMENT_STAGE);

    Renderer app;
    app.loadShader(vertexShader);
    app.loadShader(fragmentShader);

    Mesh    mesh("models/viking_room.obj");
    Texture texture("textures/viking_room.png");
    Model   model(mesh, texture);

    app.setModel(model);

    app.setInterfaceCallback(interfaceCallBack);

    try 
    {
        app.run();
    }
    catch(std::exception &e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}