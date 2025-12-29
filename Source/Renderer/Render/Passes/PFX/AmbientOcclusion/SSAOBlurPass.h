#pragma once

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct SSAOBlurPassData
	{
		rg::RGTextureDepthStencilReadId Depth;
		rg::RGTextureWriteId AOTarget;
		rg::RGTextureWriteId TempBlurTarget;
		rg::RGBufferUploadId WeightsBuffer;
	};

	class SSAOBlurPass : public rg::RenderPass<SSAOBlurPassData>
	{
	public:
		SSAOBlurPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
		dx12::PipelineState _SSAOBlurHorizonralPipeline;
		dx12::PipelineState _SSAOBlurVerticalPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
