#include "RendererPCH.h"

#include "SSAOApplyPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Render/RenderSettings.h"
#include "Render/Passes/PassResources.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

#include <random>

namespace render
{
    using namespace DirectX;

    SSAOApplyPass::SSAOApplyPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<SSAOApplyPassData>("SSAO Apply Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _SSAOPipeline.Parse("PipelineDescriptions\\SSAOApplyPipeline.tech");
    }

    void SSAOApplyPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.AOTarget = builder.ReadResource("AO Target");
        _data.HDRTarget = builder.WriteResource(HDR_TARGET);
    }

    void SSAOApplyPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("SSAO apply pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "SSAO Apply");
        {
            std::shared_ptr<dx12::Resource> aoTarget = context.GetResource(_data.AOTarget);
            std::shared_ptr<dx12::Resource> hdtTarget = context.GetResource(_data.HDRTarget);

            D3D12_GPU_DESCRIPTOR_HANDLE aoTargetHandle = context.GetGPUHandle(aoTarget->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE hdrTargetHandle = context.GetGPUHandle(hdtTarget->GetAsUAV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { aoTarget,   D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { hdtTarget,  D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetPipelineState(_SSAOPipeline);

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });

            commandList.SetDescriptorTable(0, aoTargetHandle);
            commandList.SetDescriptorTable(1, hdrTargetHandle);

            XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { aoTarget,   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { hdtTarget,  D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render