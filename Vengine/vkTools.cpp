#include "vkTools.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "libraries/tiny_obj_loader.h"

void loadModel(std::string           path,
               std::vector<Vertex> &vertices,
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
                    // attrib.vertices представляет вершины ввиде сплошного потока чисел, которые нужно привести к виду
                    // в котором предстает glm::vec3. Т.е. нарезать этот непрерывный массив числен на тройки
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                     attrib.texcoords[2 * index.texcoord_index + 0],
                     // в OBJ 0 это низ, в то время как в Vulkan 0 это верх
                     1.0f - attrib.texcoords[2 * index.texcoord_index + 1] 
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
        }
}