#include "sde_model.h"

namespace sde {
    std::vector<vk::VertexInputBindingDescription> SdeModel::Vertex::getBindingDescriptions()
    {
        std::vector<vk::VertexInputBindingDescription> bindings(1);

        bindings[0].binding = 0;
        bindings[0].stride = sizeof(Vertex);
        bindings[0].inputRate = vk::VertexInputRate::eVertex;

        return bindings;
    }

    std::vector<vk::VertexInputAttributeDescription> SdeModel::Vertex::getAttributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attributes = {};

        attributes.push_back({ 0, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, pos) });
        attributes.push_back({ 1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color) });

        return attributes;
    }
}
