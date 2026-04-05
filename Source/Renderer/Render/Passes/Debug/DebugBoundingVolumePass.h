#pragma once

#include "RenderGraph/RenderPass.h"

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct DebugBoundingVolumePassData
	{
		rg::RGTextureRenderTargetId Target;
		rg::RGTextureDepthStencilWriteId Depth;
	};

	class DebugBoundingVolumePass : public rg::RenderPass<DebugBoundingVolumePassData>
	{
	public:
		DebugBoundingVolumePass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
