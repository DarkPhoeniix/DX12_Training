#include "RendererPCH.h"

#include "GeometryPass.h"

#include "ResourceTable.h"

#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/RenderHelpers.h"
#include "Utility/DebugInfo.h"

namespace
{
    void DrawEntity(std::shared_ptr<scene::Entity> entity, dx12::CommandList& commandList, Frame* frame)
    {
        if (scene::Mesh* mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
        {
            scene::Animation* animation = entity->GetComponentAs<scene::Animation>("Animation");
            scene::Armature* armature = entity->GetComponentAs<scene::Armature>("Armature");
            scene::Transformation transform = entity->GetGlobalTransform();
            scene::Material* material = entity->GetComponentAs<scene::Material>("Material");

            CacheGPU::DataHandle modelDescHandle = frame->GetCache().RequestPlacement(entity->GetName(), sizeof(GPUModelDesc));
            GPUModelDesc* modelDesc = (GPUModelDesc*)modelDescHandle.DataCPU;
            {
                modelDesc->Transform = transform.Transform;

                if (mesh)
                {
                    modelDesc->HasMesh = 1;
                }

                if (material)
                {
                    dx12::ResourceTable& frameTable = frame->GetResourceTable();
                    std::shared_ptr<dx12::ResourceTable> textureTable = entity->GetSceneCache()->GetTextureTable();

                    frameTable.CopyDescriptor(material->Albedo.get(), dx12::ResourceViewType::SRV, *textureTable);
                    frameTable.CopyDescriptor(material->NormalMap.get(), dx12::ResourceViewType::SRV, *textureTable);
                    frameTable.CopyDescriptor(material->Metalness.get(), dx12::ResourceViewType::SRV, *textureTable);
                    frameTable.CopyDescriptor(material->Roughness.get(), dx12::ResourceViewType::SRV, *textureTable);

                    modelDesc->AlbedoTextureIndex = frameTable.GetResourceIndex(material->Albedo.get(), dx12::ResourceViewType::SRV);
                    modelDesc->NormalMapTextureIndex = frameTable.GetResourceIndex(material->NormalMap.get(), dx12::ResourceViewType::SRV);
                    modelDesc->MetalnessTextureIndex = frameTable.GetResourceIndex(material->Metalness.get(), dx12::ResourceViewType::SRV);
                    modelDesc->RoughnessTextureIndex = frameTable.GetResourceIndex(material->Roughness.get(), dx12::ResourceViewType::SRV);
                }

                if (armature)
                {
                    modelDesc->UseSkinning = true;
                }
            }
            commandList.SetCBV(1, modelDescHandle.DataGPU);

            // Update and setup animantion
            if (armature && animation)
            {
                const std::vector<scene::Bone*>& bones = armature->GetSortedBones();

                CacheGPU::DataHandle bonesDescHandle = frame->GetCache().RequestPlacement(entity->GetName() + "_bones", sizeof(DirectX::XMMATRIX) * bones.size());
                DirectX::XMMATRIX* data = (DirectX::XMMATRIX*)bonesDescHandle.DataCPU;

                for (int i = 0; i < bones.size(); ++i)
                {
                    data[i] = bones[i]->Offset * bones[i]->GlobalTransform;
                }

                commandList.SetSRV(3, bonesDescHandle.DataGPU);
            }


            commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList.SetVertexBuffer(0, mesh->VertexBufferView);
            if (!mesh->SkinningVertexData.empty())
            {
                commandList.SetVertexBuffer(1, mesh->SkinningVertexBufferView);
            }
            commandList.SetIndexBuffer(mesh->IndexBufferView);

            commandList.DrawIndexed(mesh->IndexData.size());
        }

        for (std::shared_ptr<scene::Entity>& child : entity->GetChildrenNodes())
        {
            DrawEntity(child, commandList, frame);
        }
    }
} // namespace unnamed

namespace render
{
    void GeometryPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "GeometryPass";

        _geometryPipeline.Parse("PipelineDescriptions\\GPassPipeline.tech");
    }

    void GeometryPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void GeometryPass::Execute()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_geometryPipeline);
        task->SetName("g-pass");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Geometry pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Geometry Pass");
        {
            dx12::ResourceTable& gBufferTable = _gBuffer->GetResourceTable();
            dx12::ResourceTable& frameTable = _frame->GetResourceTable();

            D3D12_CPU_DESCRIPTOR_HANDLE albedoMetalnessHandle = gBufferTable.GetResourceCPUHandle(&_gBuffer->GetAlbedoMetalnessTexture(), dx12::ResourceViewType::RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE normalSpecularHandle = gBufferTable.GetResourceCPUHandle(&_gBuffer->GetNormalTexture(), dx12::ResourceViewType::RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = gBufferTable.GetResourceCPUHandle(&_gBuffer->GetDepthTexture(), dx12::ResourceViewType::DSV);

            commandList.SetPipelineState(_geometryPipeline);

            commandList.SetViewport(_activeCamera->GetViewport());
            commandList.SetRenderTargets({ albedoMetalnessHandle, normalSpecularHandle }, &depthHandle);

            DebugInfo::StartStatCollecting(commandList);

            helpers::SetupSceneDataGPU(*_scene, commandList, _frame);

            // Setup textures
            _frame->BindDescriptorHeaps(commandList);
            commandList.SetDescriptorTable(4, frameTable.GetDescriptorHeap(dx12::ResourceViewType::SRV).GetHeapStartGPUHandle());

            for (std::shared_ptr<scene::Entity>& node : _scene->GetRootNodes())
            {
                DrawEntity(node, commandList, _frame);
            }

            DebugInfo::EndStatCollecting(commandList);

            commandList.TransitionBarrier(_gBuffer->GetDepthTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(_gBuffer->GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(_gBuffer->GetNormalTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
