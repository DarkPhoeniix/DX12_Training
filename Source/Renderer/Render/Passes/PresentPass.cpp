#include "RendererPCH.h"

#include "PresentPass.h"

#include "CommandList.h"

#include "Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	PresentPass::PresentPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<PresentPassData>("Present Pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
	}

	void PresentPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.RenderTarget = builder.CopySrcTexture("render_target");
	}

	void PresentPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("Present pass command list");

		PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Present Pass");
		{
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.RenderTarget);
            std::shared_ptr<dx12::Resource> swapChainTexture = dx12::Device::GetBackBuffer();

            commandList.TransitionBarrier(*swapChainTexture, D3D12_RESOURCE_STATE_COPY_DEST);
            commandList.CopyResource(*target, *swapChainTexture);
            commandList.TransitionBarrier(*swapChainTexture, D3D12_RESOURCE_STATE_PRESENT);
		}
		PIXEndEvent(commandList.GetDXCommandList().Get());

		commandList.Close();
	}
} // namespace render
