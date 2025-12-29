#pragma once

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct DebugRoughnessViewPassData
	{
		rg::RGTextureReadId NormalRoughness;
		rg::RGTextureWriteId Target;
	};

	class DebugRoughnessViewPass : public rg::RenderPass<DebugRoughnessViewPassData>
	{
	public:
		DebugRoughnessViewPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
		dx12::PipelineState _debugRoughnessViewPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
