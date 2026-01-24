#pragma once

#include "RenderGraph/RenderPass.h"

#include "RHI/PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
	struct GeometryPassData
	{
		rg::RGTextureDepthStencilWriteId Depth;
		rg::RGTextureRenderTargetId AlbedoMetallic;
		rg::RGTextureRenderTargetId NormalRoughness;
		rg::RGTextureRenderTargetId Emission;
	};

	class GeometryPass : public rg::RenderPass<GeometryPassData>
	{
	public:
		GeometryPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, TaskGPU& task) override;

	private:
        void _SetupPipelineState(rg::RenderContext& context, dx12::CommandList& commandList);
        void _CullPassEntities(std::vector<std::shared_ptr<scene::Entity>>& entities);
        void _PopulateDrawCommands(dx12::CommandList& commandList, const std::vector<std::shared_ptr<scene::Entity>>& entities);

		dx12::PipelineState _geometryPipeline;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
