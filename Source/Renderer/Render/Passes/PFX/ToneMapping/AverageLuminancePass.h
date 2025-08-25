#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct AverageLuminancePassData
	{
		rg::ResourceId LuminanceHistogram;
		rg::ResourceId PrevAverageLuminance;
		rg::ResourceId AverageLuminance;
	};

	class AverageLuminancePass : public rg::RenderPass<AverageLuminancePassData>
	{
	public:
		AverageLuminancePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
		dx12::PipelineState _averageLuminancePipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render

class AverageLuminancePass
{
};

