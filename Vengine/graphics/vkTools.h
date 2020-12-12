#pragma once

#include "vulkanWrapper/Vertex.h"

#include <vector>
#include <string>

namespace vkTools
{
    void loadMesh(std::string           path,
                  std::vector<Vertex>   &vertices,
                  std::vector<uint32_t> &indices);


    unsigned char *loadImage(std::string  path,
                             int          &loadedWidth,
                             int          &loadedHeight,
                             int          &loadedChannels);  

    std::vector<char> loadShader(const std::string &filename);
}