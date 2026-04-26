#pragma once

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct ToneMappingPassData
	{
		rg::RGTextureReadId HDRTarget;
		rg::RGBufferReadId AverageLuminance;

		rg::RGTextureWriteId Target;
	};

	class ToneMappingPass : public rg::RenderPass<ToneMappingPassData>
	{
	public:
		ToneMappingPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		std::unique_ptr<rhi::PipelineState> _toneMappingPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
