#include "Model.h"

Model::Model(Mesh mesh, Texture texture)
{
    position = glm::vec3(0, 0, 0);
    rotation = glm::vec3(0, 0, 0);
    scale    = glm::vec3(1, 1, 1);

    this->mesh    = mesh;
    this->texture = texture;
}