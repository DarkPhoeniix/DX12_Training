#pragma once

#include "CommandSignature.h"

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct ShadowDrawPassData
	{
		rg::RGResourceId ShadowMaps;
		std::vector<rg::RGResourceId> LightCommandBuffers[dx12::BACK_BUFFER_COUNT];
	};

	class ShadowDrawPass : public rg::RenderPass<ShadowDrawPassData>
	{
	public:
		ShadowDrawPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
		void DrawSpotLightShadows(rg::RenderContext& context, TaskGPU& task);
		void DrawPointLightShadows(rg::RenderContext& context, TaskGPU& task);

		dx12::PipelineState _spotLightShadowsPipeline;
		dx12::PipelineState _pointLightShadowsPipeline;

        dx12::CommandSignature _cmdSignature;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
