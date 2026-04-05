#pragma once

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct AmbientLightingPassData
	{
		rg::RGTextureReadId AlbedoMetallic;
		rg::RGTextureReadId NormalRoughness;
		rg::RGTextureReadId Depth;

		rg::RGTextureReadId DiffuseIrradianceMap;
		rg::RGTextureReadId PreFilteredMap;
		rg::RGTextureReadId BRDF_LUT;

		rg::RGTextureWriteId HDRTarget;
	};

	class AmbientLightingPass : public rg::RenderPass<AmbientLightingPassData>
	{
	public:
		AmbientLightingPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		std::unique_ptr<rhi::PipelineState> _ambientLightingPipeline;
        bool _useIBL;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render