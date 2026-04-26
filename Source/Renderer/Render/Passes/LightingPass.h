#pragma once

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct LightingPassData
	{
		std::vector<rg::RGVirtualResourceReadId> ShadowMaps;

		rg::RGTextureReadId AlbedoMetallic;
		rg::RGTextureReadId NormalRoughness;
		rg::RGTextureReadId Emission;
		rg::RGTextureDepthStencilReadId Depth;

		rg::RGTextureWriteId HDRTarget;
	};

	class LightingPass : public rg::RenderPass<LightingPassData>
	{
	public:
		LightingPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		std::unique_ptr<rhi::PipelineState> _deferredPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
