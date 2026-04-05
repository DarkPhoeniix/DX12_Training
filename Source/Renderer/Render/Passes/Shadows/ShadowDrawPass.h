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
		std::vector<rg::RGBufferIndirectArgsId> LightCommandBuffers;
	};

	class ShadowDrawPass : public rg::RenderPass<ShadowDrawPassData>
	{
	public:
		ShadowDrawPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		void DrawSpotLightShadows(rg::RenderContext& context, rg::ITask* task);
		void DrawPointLightShadows(rg::RenderContext& context, rg::ITask* task);

		std::unique_ptr<rhi::PipelineState> _spotLightShadowsPipeline;
		std::unique_ptr<rhi::PipelineState> _pointLightShadowsPipeline;

        std::unique_ptr<rhi::CommandSignature> _cmdSignature;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
