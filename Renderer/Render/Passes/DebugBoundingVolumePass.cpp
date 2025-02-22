#include "RendererPCH.h"

#include "DebugBoundingVolumePass.h"

#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Volumes/AABBVolume.h"
#include "Scene/Volumes/OBBVolume.h"

#include "Render/Helpers/DrawHelpers.h"

using namespace DirectX;

namespace render
{
    void DebugBoundingVolumePass::Initialize()
    {
        IRenderPass::Initialize();

        _name = "DebugBoundingVolumePass";

        _AABBpipeline.Parse("PipelineDescriptions\\AABBRenderPipeline.tech");
        _OBBpipeline.Parse("PipelineDescriptions\\OBBRenderPipeline.tech");
    }

    void DebugBoundingVolumePass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void DebugBoundingVolumePass::Execute()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_OBBpipeline);
        task->SetName("aabb");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Debug volumes command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 8, "AABB");
        {
            commandList.SetPipelineState(_OBBpipeline);

            std::vector<scene::OBBVolume> volumes;

            for (auto& node : _scene->GetRootNodes())
            {
                scene::Armature* arm = node->GetComponentAs<scene::Armature>("Armature");
                scene::Transformation* transform = node->GetComponentAs<scene::Transformation>("Transformation");

                if (arm)
                {
                    for (const auto& bone : arm->GetSortedBones())
                    {
                        DirectX::XMMATRIX boneOBB = bone->AABB.Bounds;
                        boneOBB *= bone->Offset * bone->GlobalTransform * transform->Transform;

                        scene::OBBVolume obb;
                        obb.Bounds = boneOBB;

                        volumes.push_back(obb);
                    }
                }
            }

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::ResourceTable& gBufferTable = _gBuffer->GetResourceTable();

            D3D12_CPU_DESCRIPTOR_HANDLE rtv = frameTable.GetResourceCPUHandle(&_frame->GetTargetTexture(), dx12::ResourceViewType::RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = gBufferTable.GetResourceCPUHandle(&_gBuffer->GetDepthTexture(), dx12::ResourceViewType::DSV);

            commandList.SetViewport(_activeCamera->GetViewport());
            commandList.SetRenderTarget(&rtv, &dsv);

            DirectX::XMMATRIX vp = _activeCamera->ViewProjection();

            scene::AABBVolume aabb = CombineOBBs(volumes);

            DirectX::XMVECTOR center = (aabb.Max - aabb.Min) * 0.5f;
            DirectX::XMVECTOR translation = (aabb.Max + aabb.Min) * 0.5f;
            DirectX::XMMATRIX obb = DirectX::XMMatrixScalingFromVector(center) * DirectX::XMMatrixTranslationFromVector(translation);

            commandList.SetConstants(0, 16, &vp);
            commandList.SetConstants(1, 16, &obb);

            commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

            //commandList.Draw(1);

            commandList.TransitionBarrier(_gBuffer->GetDepthTexture(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
            commandList.TransitionBarrier(_gBuffer->GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList.TransitionBarrier(_gBuffer->GetNormalTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);

            DrawHelper::DrawSphere(commandList, *_activeCamera, 80.0f, DirectX::XMVectorSet(-15.0f, 15.0f, 10.0f, 1.0f));
            //DrawHelper::DrawCone(commandList, *_activeCamera, 29.0f, 100.0f, DirectX::XMVectorSet(0.0f, 50.0f, 10.0f, 1.0f), DirectX::XMVectorSet(0.0f, -0.9f, -0.2f, 0.0f), DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
            //DrawHelper::DrawCone(commandList, *_activeCamera, 29.0f, 60.0f, DirectX::XMVectorSet(0.0f, 20.0f, 25.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));

            commandList.TransitionBarrier(_gBuffer->GetDepthTexture(), D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(_gBuffer->GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(_gBuffer->GetNormalTexture(), D3D12_RESOURCE_STATE_COMMON);

        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
