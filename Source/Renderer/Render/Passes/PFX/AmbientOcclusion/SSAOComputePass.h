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
		SSAOComputePass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		std::unique_ptr<rhi::PipelineState> _SSAOPipeline;

		std::shared_ptr<rhi::Buffer> _noise;
		std::shared_ptr<rhi::Buffer> _kernels;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render