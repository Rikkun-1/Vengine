#pragma once

#include "vulkanWrapper/Vertex.h"

#include <vector>
#include <string>

#include "pixel.h"

namespace vkTools
{
    void loadMesh(std::string            path,
                  std::vector<Vertex>   &vertices,
                  std::vector<uint32_t> &indices);

    void loadImage(const std::string  path,
                   int                &loadedWidth,
                   int                &loadedHeight,
                   int                &loadedChannels,
                   std::vector<Pixel> &pixels);

    std::vector<char> loadShader(const std::string &filename);
}