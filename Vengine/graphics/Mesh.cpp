#include "Mesh.h"

#include "vkTools.h"

Mesh::Mesh(std::string path)
{
    loadFromFile(path);
}

void Mesh::loadFromFile(std::string path)
{
    vkTools::loadMesh(path, vertices, indices);
}
