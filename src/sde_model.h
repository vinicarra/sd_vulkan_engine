#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "sde_device.h"

#include <vulkan/vulkan.hpp>
#include <vector>

namespace sde {

	class SdeModel {
	public:
		struct Vertex {
			glm::vec3 pos;
			glm::vec3 color;

			static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions();
			static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();
;		};

		struct Builder {
			std::vector<Vertex> vertices = {};
			std::vector<uint32_t> indices = {};

			void loadModel(const std::string& filePath);
		};

		SdeModel(SdeDevice& device, const Builder& builder);
		~SdeModel();

		SdeModel(const SdeModel&) = delete;
		SdeModel& operator=(const SdeModel&) = delete;

		void bind(vk::CommandBuffer commandBuffer);
		void draw(vk::CommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		uint32_t m_VertexCount = 0, m_IndexCount = 0;
		bool hasIndexBuffer = false;

		vk::UniqueBuffer m_VertexBuffer, m_IndexBuffer;
	};

}