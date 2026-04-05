#pragma once

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct ShadowCullPassData
	{
		rg::RGResourceId CounterResetBuffer;
		rg::RGResourceId AABBBuffer;
		std::vector<rg::RGResourceId> CandidateInstancesBuffer;
		std::vector<rg::RGResourceId> LightCommandBuffers;
	};

	class ShadowCullPass : public rg::RenderPass<ShadowCullPassData>
	{
	public:
		ShadowCullPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		std::unique_ptr<rhi::PipelineState> _cullShadowsPipeline;

		ComPtr<ID3D12CommandSignature> _cmdSignature;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
