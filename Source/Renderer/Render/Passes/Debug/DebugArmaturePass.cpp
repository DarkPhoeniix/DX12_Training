#include "RendererPCH.h"

#include "DebugArmaturePass.h"

#include "CommandList.h"

#include "Render/Passes/PassResources.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Transformation.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

#include "ResourceBarrier.h"

namespace render
{
    DebugArmaturePass::DebugArmaturePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<DebugArmaturePassData>("Debug Armature Pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
        _debugArmaturePipeline.Parse("PipelineDescriptions\\ArmatureDEbugPipeline.tech");
    }

    void DebugArmaturePass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.Target = builder.WriteResource(TARGET);
        _data.Depth = builder.ReadResource(DEPTH);
    }

    void DebugArmaturePass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Render Debug Armature command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 0, "Debug Armature");
        {
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

            D3D12_CPU_DESCRIPTOR_HANDLE rtv = context.GetCPUHandle(target->GetAsRTV());
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = context.GetCPUHandle(depth->GetAsDSV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { target, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET },
                { depth,  D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetPipelineState(_debugArmaturePipeline);

            commandList.SetViewport(_camera->GetViewport());
            commandList.SetRenderTarget(&rtv, &dsv);

            for (auto& node : _scene->GetRootNodes())
            {
                std::shared_ptr<scene::Armature> arm = node->GetComponentAs<scene::Armature>("Armature");
                std::shared_ptr<scene::Transformation> transform = node->GetComponentAs<scene::Transformation>("Transformation");

                if (arm)
                {
                    DirectX::XMMATRIX vp = _camera->ViewProjection();
                    DirectX::XMVECTOR* data = arm->BoneDebugTransforms->Map<DirectX::XMVECTOR>();

                    commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

                    commandList.SetConstants(0, 16, &vp);
                    commandList.SetSRV(2, arm->BoneDebugTransforms->OffsetGPU(0));

                    const auto& sortedBones = arm->GetSortedBones();
                    int ind = 0;
                    for (const auto& bone : sortedBones)
                    {
                        for (const auto& child : bone->Children)
                        {
                            data[0] = DirectX::XMVector4Transform(bone->GlobalTransform.r[3], transform->Transform);
                            data[1] = DirectX::XMVector4Transform(child->GlobalTransform.r[3], transform->Transform);

                            commandList.SetConstants(1, 4, &data[0]);
                            commandList.SetConstants(1, 4, &data[1], 4);

                            commandList.Draw(1);
                        }
                    }
                }
            }

            barriers =
            {
                { target, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON},
                { depth,  D3D12_RESOURCE_STATE_DEPTH_WRITE,   D3D12_RESOURCE_STATE_COMMON}
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
