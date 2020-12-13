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

#include "graphics/vkTools.h"

int main() 
{
    Mesh    mesh("models/suzanne.obj");
    Texture texture("textures/shapes.jpg");

    Model model(mesh, texture);

    Shader vertexShader("shaders/bin/vert.spv", ShaderStages::VERTEX_STAGE);
    Shader fragmentShader("shaders/bin/frag.spv", ShaderStages::FRAGMENT_STAGE);
        
    //for(int x = 0; x < 1000; x++)
    //   texture.setPixel(x, x, Pixel{255, 255, 255});
  

    Texture texture1px;
    //texture1px.pixels.push_back(Pixel{222, 93, 148, 255});
    //texture1px.pixels.push_back(Pixel{255, 172, 78, 255});
    texture1px.pixels.push_back(Pixel{222, 93, 96, 255});
    texture1px.width = 1;
    texture1px.height = 1;
    texture1px.channels = 4;
    
    Model modelNoTexture(mesh, texture);

    Renderer app;
    app.loadShader(vertexShader);
    app.loadShader(fragmentShader);
    app.setModel(modelNoTexture);

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