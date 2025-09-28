#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct LightingPassData
	{
		rg::RGTextureReadId AlbedoMetallic;
		rg::RGTextureReadId NormalRoughness;
		rg::RGTextureReadId Emission;
		rg::RGTextureDepthStencilReadId Depth;

		rg::RGVirtualResourceReadId ShadowMaps;

		rg::RGTextureWriteId HDRTarget;
	};

	class LightingPass : public rg::RenderPass<LightingPassData>
	{
	public:
		LightingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
		dx12::PipelineState _deferredPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
