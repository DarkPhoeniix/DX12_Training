#include "RendererPCH.h"

#include "ClearBuffersPass.h"

#include "CommandList.h"
#include "Render/Frame/TaskGPU.h"

namespace render
{
    void ClearBuffersPass::Initialize()
    {
        IRenderPass::Initialize();

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
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Clear buffers command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Clean");
        {
            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::ResourceTable& gBufferTable = _gBuffer->GetResourceTable();

            D3D12_CPU_DESCRIPTOR_HANDLE rtv = frameTable.GetResourceCPUHandle(&_frame->GetTargetTexture(), dx12::ResourceViewType::RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = gBufferTable.GetResourceCPUHandle(&_gBuffer->GetDepthTexture(), dx12::ResourceViewType::DSV);

            commandList.TransitionBarrier(_frame->GetTargetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList.TransitionBarrier(_gBuffer->GetDepthTexture(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
            commandList.TransitionBarrier(_gBuffer->GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList.TransitionBarrier(_gBuffer->GetNormalTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);

            FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

            commandList.ClearRTV(rtv, clearColor);
            commandList.ClearDSV(dsv, D3D12_CLEAR_FLAG_DEPTH);

            _gBuffer->ClearTextures(commandList);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
