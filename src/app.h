#pragma once

#include "sde_window.h"
#include "sde_device.h"
#include "sde_renderer.h"
#include "sde_model.h"
#include "sde_pipeline.h"
#include "sde_descriptors.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace sde {

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 model{ 1.f };
	};

	class App {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		App();
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void run();

	private:
		void initUBO();

	private:
		SdeWindow m_SdeWindow{WIDTH, HEIGHT, "Application"};
		SdeDevice m_SdeDevice{m_SdeWindow};
		SdeRenderer m_SdeRenderer{ m_SdeWindow, m_SdeDevice };

		vk::PipelineLayout m_PipelineLayout;

		std::shared_ptr<SdePipeline> m_DefaultPipeline;
		std::unique_ptr<SdeModel> m_TriangleModel, m_RectangleModel;

		// TODO: Remove this from here
		std::unique_ptr<SdeDescriptorSetLayout> m_DescriptorSetLayout;
		std::unique_ptr<SdeDescriptorPool> m_GlobalPool;
		std::vector<std::unique_ptr<SdeBuffer>> m_UboBuffers;
		std::vector<vk::DescriptorSet> m_DescriptorSets;
	};

}