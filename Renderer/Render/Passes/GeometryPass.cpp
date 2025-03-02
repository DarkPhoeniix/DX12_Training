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

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

namespace
{
    void DrawEntity(std::shared_ptr<scene::Entity> entity, dx12::CommandList& commandList, CacheGPU* cache, dx12::ResourceTable* resourceTable)
    {
        if (scene::Mesh* mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
        {
            scene::Animation* animation = entity->GetComponentAs<scene::Animation>("Animation");
            scene::Armature* armature = entity->GetComponentAs<scene::Armature>("Armature");
            scene::Transformation transform = entity->GetGlobalTransform();
            scene::Material* material = entity->GetComponentAs<scene::Material>("Material");

            CacheGPU::DataHandle modelDescHandle = cache->RequestPlacement(entity->GetName(), sizeof(GPUModelDesc));
            GPUModelDesc* modelDesc = (GPUModelDesc*)modelDescHandle.DataCPU;
            {
                modelDesc->Transform = transform.Transform;

                if (mesh)
                {
                    modelDesc->HasMesh = 1;
                }

                if (material)
                {
                    dx12::ResourceTable& frameTable = *resourceTable;
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

                CacheGPU::DataHandle bonesDescHandle = cache->RequestPlacement(entity->GetName() + "_bones", sizeof(DirectX::XMMATRIX) * bones.size());
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
            DrawEntity(child, commandList, cache, resourceTable);
        }
    }
} // namespace unnamed

namespace render
{
    GeometryPass::GeometryPass(scene::Scene* scene, scene::Camera* camera)
        : RenderPass<GeometryPassData>("Geometry Pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
        _geometryPipeline.Parse("PipelineDescriptions\\GPassPipeline.tech");
    }

    void GeometryPass::Setup(rg::RenderPassBuilder& builder)
    {
        dx12::ResourceDescription depthDesc;
        {
            D3D12_CLEAR_VALUE clearValue;
            clearValue.Format = DXGI_FORMAT_D32_FLOAT;
            clearValue.DepthStencil.Depth = 1;
            clearValue.DepthStencil.Stencil = 0;

            depthDesc.SetSize(_camera->GetViewport().GetSize());
            depthDesc.SetFormat(DXGI_FORMAT_D32_FLOAT);
            depthDesc.SetClearValue(clearValue);
            depthDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::DepthStencil);
        }
        _data.Depth = builder.CreateResource("Depth", depthDesc);

        dx12::ResourceDescription albedoMetallicDesc;
        {
            D3D12_CLEAR_VALUE clearValue;
            clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            clearValue.Color[0] = 0.0f;
            clearValue.Color[1] = 0.0f;
            clearValue.Color[2] = 0.0f;
            clearValue.Color[3] = 0.0f;

            albedoMetallicDesc.SetSize(_camera->GetViewport().GetSize());
            albedoMetallicDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            albedoMetallicDesc.SetClearValue(clearValue);
            albedoMetallicDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget);
        }
        _data.AlbedoMetallic = builder.CreateResource("AlbedoMetallic", albedoMetallicDesc);

        dx12::ResourceDescription normalRoughnessDesc;
        {
            D3D12_CLEAR_VALUE clearValue;
            clearValue.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            clearValue.Color[0] = 0.0f;
            clearValue.Color[1] = 0.0f;
            clearValue.Color[2] = 0.0f;
            clearValue.Color[3] = 0.0f;

            normalRoughnessDesc.SetSize(_camera->GetViewport().GetSize());
            normalRoughnessDesc.SetFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
            normalRoughnessDesc.SetClearValue(clearValue);
            normalRoughnessDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget);
        }
        _data.NormalRoughness = builder.CreateResource("NormalRoughness", normalRoughnessDesc);
    }

    void GeometryPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Geometry pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Geometry Pass");
        {
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);
            std::shared_ptr<dx12::Resource> albedoMetallic = context.GetResource(_data.AlbedoMetallic);
            std::shared_ptr<dx12::Resource> normalRoughness = context.GetResource(_data.NormalRoughness);

            dx12::ResourceTable* resourceTable = context.GetResourceTable();

            D3D12_CPU_DESCRIPTOR_HANDLE albedoMetallicHandle = resourceTable->GetResourceCPUHandle(albedoMetallic.get(), dx12::ResourceViewType::RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE normalSpecularHandle = resourceTable->GetResourceCPUHandle(normalRoughness.get(), dx12::ResourceViewType::RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = resourceTable->GetResourceCPUHandle(depth.get(), dx12::ResourceViewType::DSV);

            commandList.SetPipelineState(_geometryPipeline);

            commandList.SetViewport(_camera->GetViewport());
            commandList.SetRenderTargets({ albedoMetallicHandle, normalSpecularHandle }, &depthHandle);

            DebugInfo::StartStatCollecting(commandList);

            helpers::SetupSceneDataGPU(*_scene, commandList, context.GetCache());

            // Setup textures
            commandList.SetDescriptorTable(4, resourceTable->GetDescriptorHeap(dx12::ResourceViewType::SRV).GetHeapStartGPUHandle());

            for (std::shared_ptr<scene::Entity>& node : _scene->GetRootNodes())
            {
                DrawEntity(node, commandList, context.GetCache(), resourceTable);
            }

            DebugInfo::EndStatCollecting(commandList);

            commandList.TransitionBarrier(*depth, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*albedoMetallic, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*normalRoughness, D3D12_RESOURCE_STATE_COMMON);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
