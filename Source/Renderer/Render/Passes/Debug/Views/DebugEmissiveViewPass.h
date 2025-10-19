#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct DebugEmissiveViewPassData
	{
		rg::RGTextureReadId Emission;
		rg::RGTextureWriteId Target;
	};

	class DebugEmissiveViewPass : public rg::RenderPass<DebugEmissiveViewPassData>
	{
	public:
		DebugEmissiveViewPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
		dx12::PipelineState _debugEmissiveViewPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
