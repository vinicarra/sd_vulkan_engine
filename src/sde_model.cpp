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

    void SdeModel::bind(vk::CommandBuffer commandBuffer)
    {
        vk::Buffer buffers[] = { m_VertexBuffer->getBuffer() };
        vk::DeviceSize offsets[] = { 0 };

        commandBuffer.bindVertexBuffers(0, 1, buffers, offsets);

        if (m_HasIndexBuffer) {
            commandBuffer.bindIndexBuffer(m_IndexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
        }
    }

    void SdeModel::draw(vk::CommandBuffer commandBuffer)
    {
        if (m_HasIndexBuffer) {
            commandBuffer.drawIndexed(m_IndexCount, 1, 0, 0, 0);
        }
        else {
            commandBuffer.draw(m_VertexCount, 1, 0, 0);
        }
    }

    void SdeModel::createVertexBuffers(const std::vector<Vertex>& vertices)
    {
        m_VertexCount = static_cast<uint32_t>(vertices.size());

        uint32_t vertexSize = sizeof(vertices[0]);
        uint64_t bufferSize = static_cast<uint64_t>(vertexSize) * m_VertexCount;

        SdeBuffer stagingBuffer(m_Device, bufferSize,
            vk::BufferUsageFlagBits::eTransferSrc, 
            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped);

        stagingBuffer.map(); // This is not needed
        stagingBuffer.writeTo((void*)vertices.data());

        m_VertexBuffer = std::make_unique<SdeBuffer>(m_Device, bufferSize,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);

        m_Device.copyBuffer(stagingBuffer.getBuffer(), m_VertexBuffer->getBuffer(), bufferSize);
    }

    void SdeModel::createIndexBuffers(const std::vector<uint32_t>& indices)
    {
        m_HasIndexBuffer = indices.size() > 0;
        m_IndexCount = indices.size();

        if (!m_HasIndexBuffer) return;

        uint32_t indexSize = sizeof(indices[0]);
        uint64_t bufferSize = static_cast<uint64_t>(indexSize) * m_IndexCount;

        SdeBuffer stagingBuffer(m_Device, bufferSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped);

        stagingBuffer.map(); // This is not needed
        stagingBuffer.writeTo((void*)indices.data());

        m_IndexBuffer = std::make_unique<SdeBuffer>(m_Device, bufferSize,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);

        m_Device.copyBuffer(stagingBuffer.getBuffer(), m_IndexBuffer->getBuffer(), bufferSize);
    }

    void SdeModel::Builder::loadModel(const std::string& filePath)
    {
    }
}
