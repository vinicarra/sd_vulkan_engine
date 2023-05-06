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
    }

    void SdeModel::draw(vk::CommandBuffer commandBuffer)
    {
        commandBuffer.draw(m_VertexCount, 1, 0, 0);
    }

    void SdeModel::createVertexBuffers(const std::vector<Vertex>& vertices)
    {
        m_VertexCount = static_cast<uint32_t>(vertices.size());

        std::cout << "Allocating " << m_VertexCount << " vertices\n";

        uint32_t vertexSize = sizeof(vertices[0]);
        uint64_t bufferSize = static_cast<uint64_t>(vertexSize) * m_VertexCount;

        SdeBuffer stagingBuffer(m_Device, bufferSize,
            vk::BufferUsageFlagBits::eTransferSrc, 
            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped);

        stagingBuffer.map(); // This is not needed
        stagingBuffer.writeTo((void*)vertices.data(), bufferSize);

        m_VertexBuffer = std::make_unique<SdeBuffer>(m_Device, bufferSize,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);

        m_Device.copyBuffer(stagingBuffer.getBuffer(), m_VertexBuffer->getBuffer(), bufferSize);
    }

    void SdeModel::createIndexBuffers(const std::vector<uint32_t>& indices)
    {
    }

    void SdeModel::Builder::loadModel(const std::string& filePath)
    {
    }
}
