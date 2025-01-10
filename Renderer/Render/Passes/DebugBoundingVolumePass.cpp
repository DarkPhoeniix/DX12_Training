#include "RendererPCH.h"

#include "DebugBoundingVolumePass.h"

#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Volumes/AABBVolume.h"
#include "Scene/Volumes/OBBVolume.h"

using namespace DirectX;

namespace render
{
    void DebugBoundingVolumePass::Inititalize()
    {
        IRenderPass::Inititalize();

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
        task->AddDependency("armature");

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

            dx12::DescriptorHeap& RTVHeap = _frame->GetDescriptorHeap(dx12::DescriptorHeapType::RTV);

            D3D12_CPU_DESCRIPTOR_HANDLE rtv = RTVHeap.GetResourceCPUHandle(&_frame->GetTargetTexture(), dx12::ResourceViewType::RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = _gBuffer->GetDepthTextureCPUHandle();

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

            commandList.Draw(1);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
