#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct AmbientLightingPassData
	{
		rg::RGResourceId AlbedoMetallic;
		rg::RGResourceId NormalRoughness;
		rg::RGResourceId Depth;

		rg::RGResourceId DiffuseIrradianceMap;
		rg::RGResourceId PreFilteredMap;
		rg::RGResourceId BRDF_LUT;

		rg::RGResourceId HDRTarget;
	};

	class AmbientLightingPass : public rg::RenderPass<AmbientLightingPassData>
	{
	public:
		AmbientLightingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
		dx12::PipelineState _ambientLightingPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render