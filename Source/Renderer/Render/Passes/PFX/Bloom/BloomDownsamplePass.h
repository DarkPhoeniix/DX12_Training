#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct BloomDownsamplePassData
	{
        rg::ResourceId HDRTarget;
		std::vector<rg::ResourceId> BloomMips;
	};

	class BloomDownsamplePass : public rg::RenderPass<BloomDownsamplePassData>
	{
	public:
		BloomDownsamplePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
		dx12::PipelineState _bloomDownsamplePass1Pipeline;
		dx12::PipelineState _bloomDownsamplePipeline;

		std::uint32_t _mipCount;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
