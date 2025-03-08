#include "RendererPCH.h"

#include "FXAAPass.h"

#include "CommandList.h"

#include "Scene/Entity/Components/Camera.h"
#include "Render/Helpers/RenderHelpers.h"
#include "Render/Passes/PassResources.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

namespace render
{
    FXAAPass::FXAAPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<FXAAPassData>("FXAA Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _FXAAPipeline.Parse("PipelineDescriptions\\FXAAPipeline.tech");
    }

    void FXAAPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.Target = builder.WriteResource(TARGET);

        dx12::ResourceDescription targetDesc;
        {
            targetDesc.SetSize(_camera->GetViewport().GetSize());
            targetDesc.SetDimension(D3D12_RESOURCE_DIMENSION_TEXTURE2D);
            targetDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            targetDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        _data.FXAATarget = builder.CreateResource("FXAATarget", targetDesc);
    }

    void FXAAPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("FXAA command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "FXAA Pass");
        {
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);
            std::shared_ptr<dx12::Resource> fxaa = context.GetResource(_data.FXAATarget);

            D3D12_GPU_DESCRIPTOR_HANDLE targetHandle = context.GetGPUHandle(target->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE fxaaHandle = context.GetGPUHandle(fxaa->GetAsUAV());

            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(*fxaa, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            commandList.SetPipelineState(_FXAAPipeline);

            helpers::SetupSceneDataGPU(*_scene, commandList, &context.GetCache());

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });
            commandList.SetDescriptorTable(3, targetHandle);
            commandList.SetDescriptorTable(4, fxaaHandle);

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*fxaa, D3D12_RESOURCE_STATE_COMMON);

            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_COPY_DEST);
            commandList.TransitionBarrier(*fxaa, D3D12_RESOURCE_STATE_COPY_SOURCE);

            commandList.CopyResource(*fxaa, *target);

            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*fxaa, D3D12_RESOURCE_STATE_COMMON);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
