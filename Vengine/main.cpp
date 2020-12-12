#include <iostream>
#include <stdexcept>
#include <cstdlib> // макросы EXIT_SUCCESS и EXIT_FAILURE

#ifdef NDEBUG
    #define USE_VALIDATION_LAYERS
#endif

#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "graphics/Model.h"
#include "graphics/Renderer.h"
#include "graphics/Shader.h"

int main() 
{
    Mesh    mesh("models/viking_room.obj");
    Texture texture("textures/viking_room.png");

    Model model(mesh, texture);

    Shader vertexShader  ("shaders/bin/vert.spv", ShaderStages::VERTEX_STAGE);
    Shader fragmentShader("shaders/bin/frag.spv", ShaderStages::FRAGMENT_STAGE);

    Renderer app;
    app.changeModel(model);
    app.changeTexture(texture);
    app.loadShader(vertexShader);
    app.loadShader(fragmentShader);

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