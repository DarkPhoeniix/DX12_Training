#include "RendererPCH.h"

#include "PresentPass.h"

#include "Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	PresentPass::PresentPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<PresentPassData>("present_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
	}

	void PresentPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.RenderTarget = builder.CopySrcTexture("render_target");
	}

	void PresentPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
			GPU_SCOPED_EVENT(commandList, "Present Pass", 2);

            std::shared_ptr<rhi::Texture> target = context.GetTexture(_data.RenderTarget);
            std::shared_ptr<rhi::Texture> swapChainTexture = rhi::Device::GetBackBuffer();

            commandList->TransitionBarrier(swapChainTexture, rhi::ResourceState::CopyDest);
            commandList->CopyTexture(target, swapChainTexture);
            commandList->TransitionBarrier(swapChainTexture, rhi::ResourceState::Present);
		}

		commandList->Close();
	}
} // namespace render
