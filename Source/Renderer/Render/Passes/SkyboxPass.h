#pragma once

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"

namespace render
{
	struct SkyboxPassData
	{
		rg::RGTextureDepthStencilReadId Depth;
		rg::RGTextureWriteId HDRTarget;
		rg::RGTextureReadId Skybox;
	};

	class SkyboxPass : public rg::RenderPass<SkyboxPassData>
	{
	public:
		SkyboxPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		std::unique_ptr<rhi::PipelineState> _skyboxPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
