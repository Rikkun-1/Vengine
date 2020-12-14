#include "Model.h"

///////////////////////// MODEL BEG //////////////////////////////

Model::Model()
{
    position = glm::vec3(0, 0, 0);
    rotation = glm::vec3(0, 0, 0);
    scale    = glm::vec3(1.0f, 1.0f, 1.0f);
}

Model::Model(Mesh mesh, Texture texture) : Model()
{
    this->mesh    = mesh;
    this->texture = texture;
}

///////////////////////// MODEL END //////////////////////////////