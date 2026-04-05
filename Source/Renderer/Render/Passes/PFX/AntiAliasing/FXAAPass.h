#pragma once

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderPass.h"

#include "RHI/CommandSignature.h"
#include "RHI/PipelineState.h"

namespace render
{
	struct FXAAPassData
	{
		rg::RGTextureWriteId Target;
		rg::RGTextureWriteId LumaBuffer;
		rg::RGBufferWriteId WorkCounters;
		rg::RGBufferWriteId WorkQueue;
		rg::RGBufferWriteId ColorQueue;
		rg::RGBufferIndirectArgsId IndirectParams;
	};

	class FXAAPass : public rg::RenderPass<FXAAPassData>
	{
	public:
		FXAAPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

		// Inherited via RenderPass
		void Setup(rg::RenderPassBuilder& builder) override;
		void Execute(rg::RenderContext& context, rg::ITask* task) override;

	private:
		std::unique_ptr<rhi::PipelineState> _FXAA_Pass1_Pipeline;
		std::unique_ptr<rhi::PipelineState> _FXAA_ResolveWork_Pipeline;
		std::unique_ptr<rhi::PipelineState> _FXAA_Pass2H_Pipeline;
		std::unique_ptr<rhi::PipelineState> _FXAA_Pass2V_Pipeline;

		std::unique_ptr<rhi::CommandSignature> _cmdSignature;

		std::shared_ptr<rhi::Buffer> _paramsReset;

		std::shared_ptr<scene::Scene> _scene;
		scene::Camera* _camera;
	};
} // namespace render
