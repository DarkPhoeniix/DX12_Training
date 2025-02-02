#include "RendererPCH.h"

#include "FXAAPass.h"

#include "Render/Helpers/RenderHelpers.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    void FXAAPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "FXAAPass";

        _FXAAPipeline.Parse("PipelineDescriptions\\FXAAPipeline.tech");

        {
            dx12::ResourceDescription desc = {};
            desc.SetSize(_activeCamera->GetViewport().GetSize());
            desc.SetDimension(D3D12_RESOURCE_DIMENSION_TEXTURE2D);
            desc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            desc.SetResourceType(dx12::EResourceType::Texture | dx12::EResourceType::Unordered);

            _fxaaRTT.CreateCommitedResource(desc);

            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();

            sceneTable.PlaceResource(&_fxaaRTT, dx12::ResourceViewType::UAV);

        }
    }

    void FXAAPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void FXAAPass::Execute()
    {
        {
            TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
            task->SetName("transitionToFXAA");
            _tasks.push_back(task);

            dx12::CommandList& commandList = *task->GetCommandLists().front();
            commandList.SetName("Transition to FXAA command list");

            commandList.TransitionBarrier(_frame->GetTargetTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(_fxaaRTT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            commandList.Close();
        }

        {
            TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_FXAAPipeline);
            task->SetName("fxaa");
            task->AddDependency("transitionToFXAA");
            _tasks.push_back(task);

            dx12::CommandList& commandList = *task->GetCommandLists().front();
            commandList.SetName("FXAA command list");

            PIXBeginEvent(commandList.GetDXCommandList().Get(), 10, "FXAA");
            {
                commandList.SetPipelineState(_FXAAPipeline);

                dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
                dx12::ResourceTable& frameTable = _frame->GetResourceTable();

                frameTable.CopyDescriptor(&_fxaaRTT, dx12::ResourceViewType::UAV, sceneTable);

                _frame->BindDescriptorHeaps(commandList);

                D3D12_GPU_DESCRIPTOR_HANDLE targetTextureHandle = frameTable.GetResourceGPUHandle(&_frame->GetTargetTexture(), dx12::ResourceViewType::SRV);
                D3D12_GPU_DESCRIPTOR_HANDLE fxaaTextureHandle = frameTable.GetResourceGPUHandle(&_fxaaRTT, dx12::ResourceViewType::UAV);
                
                helpers::SetupSceneDataGPU(*_scene, commandList, _frame);

                commandList.SetDescriptorTable(3, targetTextureHandle);
                commandList.SetDescriptorTable(4, fxaaTextureHandle);

                DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
                int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
                int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

                commandList.Dispatch(xThreadGroups, yThreadGroups);

            }
            PIXEndEvent(commandList.GetDXCommandList().Get());

            commandList.Close();
        }

        {
            TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
            task->SetName("transitionFromFXAA");
            task->AddDependency("fxaa");
            _tasks.push_back(task);

            dx12::CommandList& commandList = *task->GetCommandLists().front();
            commandList.SetName("Transition from FXAA command list");

            commandList.TransitionBarrier(_frame->GetTargetTexture(), D3D12_RESOURCE_STATE_COPY_DEST);
            commandList.TransitionBarrier(_fxaaRTT, D3D12_RESOURCE_STATE_COPY_SOURCE);

            commandList.CopyResource(_fxaaRTT, _frame->GetTargetTexture());

            commandList.TransitionBarrier(_frame->GetTargetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList.TransitionBarrier(_fxaaRTT, D3D12_RESOURCE_STATE_COMMON);

            commandList.Close();
        }
    }
} // namespace render
