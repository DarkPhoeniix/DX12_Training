#include "RendererPCH.h"

#include "ClearBuffersPass.h"

#include "CommandList.h"
#include "Render/Frame/TaskGPU.h"

namespace render
{
    void ClearBuffersPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "ClearBuffersPass";
    }

    void ClearBuffersPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void ClearBuffersPass::Execute()
    {
        // Create task and dedicated command list
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("clean");

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Clear buffers command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Clean");
        {
            dx12::DescriptorHeap& RTVHeap = _frame->GetDescriptorHeap(dx12::DescriptorHeapType::RTV);

            D3D12_CPU_DESCRIPTOR_HANDLE rtv = RTVHeap.GetResourceCPUHandle(&_frame->GetTargetTexture(), dx12::ResourceViewType::RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = _gBuffer->GetDepthTextureCPUHandle();

            commandList.TransitionBarrier(_frame->GetTargetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);

            FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

            commandList.ClearRTV(rtv, clearColor);
            commandList.ClearDSV(dsv, D3D12_CLEAR_FLAG_DEPTH);

            _gBuffer->ClearTextures(commandList);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
