#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include "Mesh.h"
#include "Texture.h"

class Model
{
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    Mesh    mesh;
    Texture texture;

    Model();

    Model(Mesh mesh, Texture texture);
};

