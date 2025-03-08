#include "RendererPCH.h"

#include "ShadowCullPass.h"

#include "CommandList.h"

#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/RenderHelpers.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
    constexpr std::uint32_t CULLING_PASS_THREADS_NUM = 16;

    // Data structure to match the command signature used for ExecuteIndirect.
    struct IndirectCommand
    {
        D3D12_GPU_VIRTUAL_ADDRESS VertexBufferAddress;
        UINT VertexBufferSize;
        UINT VertexBufferStride;
        D3D12_GPU_VIRTUAL_ADDRESS SkinBufferAddress;
        UINT SkinBufferSize;
        UINT SkinBufferStride;
        D3D12_GPU_VIRTUAL_ADDRESS IndexBufferAddress;
        UINT IndexBufferSize;
        UINT IndexBufferFormat;
        D3D12_GPU_VIRTUAL_ADDRESS SceneBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS ModelBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS BonesBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS LightsBufferAddress;
        UINT LightIndex;

        D3D12_DRAW_ARGUMENTS DrawArguments;
    };

    std::uint32_t AlignToUAVCounterOffset(std::uint32_t size)
    {
        return Math::AlignUp(size, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
    }

    void SetupEntity(std::shared_ptr<scene::Entity> entity, dx12::CommandList& commandList, CacheGPU* cache, dx12::ResourceTable* resourceTable)
    {
        if (scene::Mesh* mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
        {
            scene::Armature* armature = entity->GetComponentAs<scene::Armature>("Armature");
            scene::Transformation transform = entity->GetGlobalTransform();
            scene::Material* material = entity->GetComponentAs<scene::Material>("Material");

            CacheGPU::DataHandle modelDescHandle = cache->GetOrPlaceResource(entity->GetName(), sizeof(GPUModelDesc));
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

            // Update and setup animantion
            if (armature)
            {
                const std::vector<scene::Bone*>& bones = armature->GetSortedBones();

                CacheGPU::DataHandle bonesDescHandle = cache->GetOrPlaceResource(entity->GetName() + "_bones", sizeof(DirectX::XMMATRIX) * bones.size());
                DirectX::XMMATRIX* data = (DirectX::XMMATRIX*)bonesDescHandle.DataCPU;

                for (int i = 0; i < bones.size(); ++i)
                {
                    data[i] = bones[i]->Offset * bones[i]->GlobalTransform;
                }
            }
        }

        for (std::shared_ptr<scene::Entity>& child : entity->GetChildrenNodes())
        {
            SetupEntity(child, commandList, cache, resourceTable);
        }
    }
} // namespace unnamed

namespace render
{
    ShadowCullPass::ShadowCullPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<ShadowCullPassData>("Shadow Culling Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _cullShadowsPipeline.Parse("PipelineDescriptions\\LightCulling_PointLight.tech");
        _lightShadowsPipeline.Parse("PipelineDescriptions\\Shadow_SpotLight.tech");

        {
            dx12::ResourceDescription counterResetBuffer;
            counterResetBuffer.SetSize({ sizeof(UINT), 1 });
            counterResetBuffer.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
            counterResetBuffer.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);

            _counterReset.CreateCommitedResource(counterResetBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
            _counterReset.SetName("Counter reset buffer");
            UINT* val = (UINT*)_counterReset.Map();
            val[0] = 0;
        }
    }

    void ShadowCullPass::Setup(rg::RenderPassBuilder& builder)
    {
        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t lightsNum = lightEntities.size();
        size_t meshesNum = meshes.size();

        dx12::ResourceDescription commandsBufferDescription;
        {
            commandsBufferDescription.SetSize({ (AlignToUAVCounterOffset(meshesNum * sizeof(IndirectCommand)) + (uint32_t)sizeof(UINT)), 1 });
            commandsBufferDescription.SetStride(sizeof(IndirectCommand));
            commandsBufferDescription.SetUAVCounterOffset(AlignToUAVCounterOffset(meshesNum * sizeof(IndirectCommand)));
            commandsBufferDescription.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
        }

        for (size_t frameIndex = 0; frameIndex < dx12::BACK_BUFFER_COUNT; ++frameIndex)
        {
            _data.LightCommandBuffers[frameIndex].resize(lightsNum);
            for (size_t i = 0; i < lightsNum; ++i)
            {
                _data.LightCommandBuffers[frameIndex][i] = builder.CreateResource(std::format("ShadowCullBuffer {} (frame {})", i, frameIndex), commandsBufferDescription);
            }
        }
    }

    void ShadowCullPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Shadow pass command list - culling");

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Shadow Pass | Culling");

        // Setup pipeline
        commandList.SetPipelineState(_cullShadowsPipeline);
        commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });

        helpers::SetupSceneDataGPU(*_scene, commandList, &context.GetCache());
        helpers::SetupLightDataGPU(*_scene, commandList, &context.GetCache(), context.GetResourceTable());

        CacheGPU::DataHandle sceneAddress = context.GetCache().GetResourcePlacement("SceneCB");
        CacheGPU::DataHandle lightsAddress = context.GetCache().GetResourcePlacement("LightsCB");

        for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

            scene::Light* light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");

            std::shared_ptr<dx12::Resource> commandBuffer = context.GetResource(_data.LightCommandBuffers[context.GetFrameIndex()][lightIndex]);

            CacheGPU::DataHandle inputCommands = context.GetCache().RequestPlacement(std::format("InputCmd_{}", lightEntities[lightIndex]->GetName()), objectsNum * sizeof(IndirectCommand));
            IndirectCommand* commandsList = (IndirectCommand*)inputCommands.DataCPU;

            // Record all draw command to the commandBuffer

            for (size_t j = 0; j < objectsNum; ++j)
            {
                scene::Mesh* mesh = meshes[j]->GetComponentAs<scene::Mesh>("Mesh");

                if (!mesh)
                {
                    continue;
                }

                SetupEntity(meshes[j], commandList, &context.GetCache(), &context.GetResourceTable());

                CacheGPU::DataHandle modelAddress = context.GetCache().GetResourcePlacement(meshes[j]->GetName());
                CacheGPU::DataHandle bonesAddress = context.GetCache().GetResourcePlacement(meshes[j]->GetName() + "_bones");

                IndirectCommand command;

                command.VertexBufferAddress = mesh->VertexBufferView.BufferLocation;
                command.VertexBufferSize = mesh->VertexBufferView.SizeInBytes;
                command.VertexBufferStride = mesh->VertexBufferView.StrideInBytes;

                if (!mesh->SkinningVertexData.empty())
                {
                    command.SkinBufferAddress = mesh->SkinningVertexBufferView.BufferLocation;
                    command.SkinBufferSize = mesh->SkinningVertexBufferView.SizeInBytes;
                    command.SkinBufferStride = mesh->SkinningVertexBufferView.StrideInBytes;

                    command.BonesBufferAddress = bonesAddress.DataGPU;
                }
                else
                {
                    command.SkinBufferAddress = mesh->VertexBufferView.BufferLocation;
                    command.SkinBufferSize = mesh->VertexBufferView.SizeInBytes;
                    command.SkinBufferStride = mesh->VertexBufferView.StrideInBytes;

                    command.BonesBufferAddress = modelAddress.DataGPU;
                }

                command.IndexBufferAddress = mesh->IndexBufferView.BufferLocation;
                command.IndexBufferSize = mesh->IndexBufferView.SizeInBytes;
                command.IndexBufferFormat = mesh->IndexBufferView.Format;

                command.SceneBufferAddress = sceneAddress.DataGPU;
                command.LightsBufferAddress = lightsAddress.DataGPU;
                command.ModelBufferAddress = modelAddress.DataGPU;
                command.LightIndex = lightIndex;

                command.DrawArguments.VertexCountPerInstance = mesh->VertexData.size();
                command.DrawArguments.InstanceCount = 1;
                command.DrawArguments.StartVertexLocation = 0;
                command.DrawArguments.StartInstanceLocation = 0;

                commandsList[j] = command;
            }

            // Write all models bounding boxes to the buffer
            CacheGPU::DataHandle AABBs = context.GetCache().RequestPlacement("AABBs", objectsNum * sizeof(DirectX::XMVECTOR) * 2);
            for (size_t j = 0, count = 0; j < objectsNum; ++j)
            {
                if (scene::Mesh* mesh = _scene->GetRootNodes()[j]->GetComponentAs<scene::Mesh>("Mesh"))
                {
                    scene::Transformation* t = _scene->GetRootNodes()[j]->GetComponentAs<scene::Transformation>("Transformation");

                    DirectX::XMVECTOR* data = (DirectX::XMVECTOR*)AABBs.DataCPU;
                    scene::AABBVolume aabb = mesh->AABB.Transform(t->Transform);

                    data[count++] = aabb.Min;
                    data[count++] = aabb.Max;
                }
            }

            // Transition resources
            commandList.TransitionBarrier(*commandBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

            // Reset commands counter
            std::uint32_t counterBufferOffset = commandBuffer->GetResourceDescription().GetSize().x - sizeof(UINT);
            commandList.CopyBufferRegion(_counterReset, *commandBuffer, sizeof(UINT), 0, counterBufferOffset);

            // Transition resources
            commandList.TransitionBarrier(*commandBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            D3D12_GPU_DESCRIPTOR_HANDLE cbHandle = context.GetGPUHandle(commandBuffer->GetAsUAV());

            // Setup root signature
            commandList.SetConstant(3, objectsNum);
            commandList.SetSRV(4, AABBs.DataGPU);
            commandList.SetSRV(5, inputCommands.DataGPU);
            commandList.SetDescriptorTable(6, cbHandle);

            // Dispatch culling compute shader
            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(objectsNum / (float)CULLING_PASS_THREADS_NUM);
            commandList.Dispatch(xThreadGroups);

            PIXEndEvent(commandList.GetDXCommandList().Get());
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
