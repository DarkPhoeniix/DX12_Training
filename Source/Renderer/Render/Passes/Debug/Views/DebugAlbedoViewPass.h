#pragma once

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct DebugAlbedoViewPassData
	{
		rg::RGTextureReadId AlbedoMetallic;
		rg::RGTextureWriteId Target;
	};

	class DebugAlbedoViewPass : public rg::RenderPass<DebugAlbedoViewPassData>
	{
	public:
		DebugAlbedoViewPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		std::unique_ptr<rhi::PipelineState> _debugAlbedoViewPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
