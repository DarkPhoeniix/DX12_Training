#include "RendererPCH.h"

#include "DebugArmaturePass.h"

#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    void DebugArmaturePass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "DebugArmaturePass";

        _debugArmaturePipeline.Parse("PipelineDescriptions\\ArmatureDebugPipeline.tech");
    }

    void DebugArmaturePass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void DebugArmaturePass::Execute()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_debugArmaturePipeline);
        task->SetName("armature");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Debug armature command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 8, "Armature");
        {
            commandList.TransitionBarrier(_gBuffer->GetDepthTexture(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
            commandList.TransitionBarrier(_gBuffer->GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList.TransitionBarrier(_gBuffer->GetNormalTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);

            commandList.SetPipelineState(_debugArmaturePipeline);

            for (auto& node : _scene->GetRootNodes())
            {
                scene::Armature* arm = node->GetComponentAs<scene::Armature>("Armature");
                scene::Transformation* transform = node->GetComponentAs<scene::Transformation>("Transformation");

                if (arm)
                {
                    dx12::ResourceTable& frameTable = _frame->GetResourceTable();
                    dx12::ResourceTable& gBufferTable = _gBuffer->GetResourceTable();

                    D3D12_CPU_DESCRIPTOR_HANDLE rtv = frameTable.GetResourceCPUHandle(&_frame->GetTargetTexture(), dx12::ResourceViewType::RTV);
                    D3D12_CPU_DESCRIPTOR_HANDLE dsv = gBufferTable.GetResourceCPUHandle(&_gBuffer->GetDepthTexture(), dx12::ResourceViewType::DSV);

                    commandList.SetViewport(_activeCamera->GetViewport());
                    commandList.SetRenderTarget(&rtv, &dsv);

                    DirectX::XMMATRIX vp = _activeCamera->ViewProjection();
                    DirectX::XMVECTOR* data = (DirectX::XMVECTOR*)arm->BoneDebugTransforms.Map();

                    const auto& sortedBones = arm->GetSortedBones();
                    int ind = 0;
                    for (const auto& bone : sortedBones)
                    {
                        for (const auto& child : bone->Children)
                        {
                            data[0] = DirectX::XMVector4Transform(bone->GlobalTransform.r[3], transform->Transform);
                            data[1] = DirectX::XMVector4Transform(child->GlobalTransform.r[3], transform->Transform);

                            commandList.SetConstants(0, 16, &vp);
                            commandList.SetConstants(1, 4, &data[0]);
                            commandList.SetConstants(1, 4, &data[1], 4);
                            commandList.SetSRV(2, arm->BoneDebugTransforms.OffsetGPU(0));

                            commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

                            commandList.Draw(1);
                        }
                    }
                }
            }
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
