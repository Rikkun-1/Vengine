#include "Mesh.h"

///////////////////////// MESH BEG //////////////////////////////

Mesh::Mesh(std::string path)
{
    loadFromFile(path);
}

void Mesh::loadFromFile(std::string path)
{
    VengineTools::loadMesh(path, vertices, indices);
}

///////////////////////// MESH END //////////////////////////////