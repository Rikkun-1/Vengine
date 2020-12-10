#pragma once

#include <string>
#include <vector>

#include "vulkanWrapper/Vertex.h"


class Mesh
{
public:
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    Mesh() = default;

    Mesh(std::string path);

    void loadFromFile(std::string path);
};

