#include "Vertex.h"

// эта функция нужна для того чтобы описать каким образом этот формат данных
// должен передаваться в вершинный шейдер
VkVertexInputBindingDescription Vertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    // описывает индекс этого прикрепления в массиве прикреплений
    // у нас вся вершина упакована в одну структуру поэтому индекс всего один
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex); // смещение
    // VK_VERTEX_INPUT_RATE_VERTEX перемещаться к следующему набору данных после каждой вершины
    // VK_VERTEX_INPUT_RATE_INSTANCE после каждого экземпляра. Требуется для реализации инстансинга
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

// эта функия описывает то каким образом данные должны приниматься вершинным шейдером
std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    // описывает из какого прикрепляения принимать данные
    attributeDescriptions[0].binding = 0; 
    //описывает к какому location вершинного шейдера прикреплять полученные данные
    attributeDescriptions[0].location = 0; 
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}