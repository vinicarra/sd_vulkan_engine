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

    SdeModel::SdeModel(SdeDevice& device, const Builder& builder) : m_Device(device)
    {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    SdeModel::~SdeModel()
    {
    }

    void SdeModel::createVertexBuffers(const std::vector<Vertex>& vertices)
    {
        m_VertexCount = static_cast<uint32_t>(vertices.size());

        uint32_t vertexSize = sizeof(vertices[0]);
        uint64_t bufferSize = static_cast<uint64_t>(vertexSize) * m_VertexCount;


    }

    void SdeModel::createIndexBuffers(const std::vector<uint32_t>& indices)
    {
    }
}
