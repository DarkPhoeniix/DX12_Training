#include "RendererPCH.h"

#include "ShadowCullPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

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

        CacheGPU::DataHandle sceneDataHandle = context.GetCache().GetResourcePlacement("SceneCB");
        commandList.SetCBV(0, sceneDataHandle.DataGPU);

        CacheGPU::DataHandle lightsData = context.GetCache().GetResourcePlacement("LightsCB");
        commandList.SetSRV(2, lightsData.DataGPU);

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

                CacheGPU::DataHandle modelAddress = context.GetCache().GetResourcePlacement(meshes[j]->GetName());
                CacheGPU::DataHandle bonesAddress = modelAddress;
                if (scene::Armature* armature = meshes[j]->GetComponentAs<scene::Armature>("Armature"))
                {
                    bonesAddress = context.GetCache().GetResourcePlacement(meshes[j]->GetName() + "_bones");
                }

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
                    DirectX::XMVECTOR* data = (DirectX::XMVECTOR*)AABBs.DataCPU;
                    scene::AABBVolume aabb = mesh->GlobalAABB;

                    data[count++] = aabb.Min;
                    data[count++] = aabb.Max;
                }
            }

            // Transition resources
            std::vector<dx12::ResourceBarrier> barriers =
            {
                { commandBuffer.get(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,    D3D12_RESOURCE_STATE_COPY_DEST }
            };
            commandList.TransitionBarriers(barriers);

            // Reset commands counter
            std::uint32_t counterBufferOffset = commandBuffer->GetResourceDescription().GetSize().x - sizeof(UINT);
            commandList.CopyBufferRegion(_counterReset, *commandBuffer, sizeof(UINT), 0, counterBufferOffset);

            // Transition resources
            barriers =
            {
                { commandBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
            };
            commandList.TransitionBarriers(barriers);

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
