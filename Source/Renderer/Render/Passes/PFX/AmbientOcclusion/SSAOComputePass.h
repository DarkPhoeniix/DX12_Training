#pragma once

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct SSAOComputePassData
	{
		rg::RGTextureReadId NormalRoughness;
		rg::RGTextureDepthStencilReadId Depth;
		rg::RGTextureWriteId AOTarget;
		rg::RGBufferUploadId Noise;
		rg::RGBufferUploadId Kernels;
	};

	class SSAOComputePass : public rg::RenderPass<SSAOComputePassData>
	{
	public:
		SSAOComputePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
		dx12::PipelineState _SSAOPipeline;

		std::shared_ptr<dx12::Resource> _noise;
		std::shared_ptr<dx12::Resource> _kernels;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render