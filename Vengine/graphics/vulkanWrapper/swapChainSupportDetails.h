#pragma once

#include "vulkanWrapper/Vertex.h"

#include <vector>
#include <string>

void loadModel(std::string           path,
               std::vector<Vertex>   &vertices,
               std::vector<uint32_t> &indices);