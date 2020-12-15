#include "tools.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "libraries/tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "libraries/stb_image.h"

///////////////////////// STATIC BEG /////////////////////////////

static std::vector<char> loadFile(const std::string &filename)
{
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if(!file.is_open())
            throw std::runtime_error("failed to open file!");

        size_t fileSize = (size_t) file.tellg();

        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
}

///////////////////////// STATIC END //////////////////////////////



///////////////////////// VK TOOLS BEG //////////////////////////////

namespace VengineTools
{
    void loadMesh(std::string            path,
                  std::vector<Vertex>   &vertices,
                  std::vector<uint32_t> &indices)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
            throw std::runtime_error(warn + err);

        for(const auto &shape : shapes)
        {
            for(const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                     attrib.texcoords[2 * index.texcoord_index + 0],
                     1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
        }
    }


    void loadImage(const std::string  path,
                   int                &loadedWidth,
                   int                &loadedHeight,
                   int                &loadedChannels,
                   std::vector<Pixel> &pixels)
    {
        stbi_uc *rawPixels = stbi_load(path.c_str(),
                                       &loadedWidth,
                                       &loadedHeight,
                                       &loadedChannels,
                                       STBI_rgb_alpha);
        
        loadedChannels = 4; // STBI_rgb_alpha имеет 4 канала

        if(!rawPixels)
            throw std::runtime_error("failed to load texture image! " + path);

        size_t rawPixelsByteSize = loadedWidth * loadedHeight * loadedChannels;

        pixels.resize(loadedWidth * loadedHeight);

        memcpy(pixels.data(), rawPixels, rawPixelsByteSize);
        stbi_image_free(rawPixels);
    }


    std::vector<char> loadShader(const std::string &filename)
    {
        return loadFile(filename);
    }
}

///////////////////////// VK TOOLS END //////////////////////////////

