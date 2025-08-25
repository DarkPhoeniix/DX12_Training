#include "RendererPCH.h"

#include "ShadowCullPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Volumes/AABBVolume.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
    constexpr std::uint32_t CULLING_PASS_THREADS_NUM = 16;
    constexpr std::uint32_t MAX_INSTANCES_NUM = 1u << 20;

    // Data structure to match the command signature used for ExecuteIndirect.
    struct alignas(16) IndirectCommand
    {
        D3D12_GPU_VIRTUAL_ADDRESS VertexBufferAddress;
        UINT VertexBufferSize;
        UINT VertexBufferStride;
        D3D12_GPU_VIRTUAL_ADDRESS SkinBufferAddress;
        UINT SkinBufferSize;
        UINT SkinBufferStride;
		//D3D12_GPU_VIRTUAL_ADDRESS IndexBufferAddress;
		//UINT IndexBufferSize;
  //      UINT IndexBufferFormat;

        D3D12_GPU_VIRTUAL_ADDRESS FrameBufferAddress;
        UINT InstanceIndex;
        UINT LightIndex;

        D3D12_DRAW_ARGUMENTS DrawArguments;
    };

    struct PassConstants
    {
        std::uint32_t LightIndex;
        std::uint32_t CommandsCount;

        std::uint32_t AABBBufferIndex;
        std::uint32_t InputCommandsBufferIndex;
        std::uint32_t OutputCommandsBufferIndex;
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
    }

    void ShadowCullPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.ShadowMaps = builder.ReadResource("shadow_maps");

        dx12::ResourceDescription counterResetBuffer;
        counterResetBuffer.SetSize({ sizeof(UINT), 1 });
        counterResetBuffer.SetStride(sizeof(UINT));
        counterResetBuffer.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
        counterResetBuffer.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);

        std::uint32_t value = 0;
		_data.CounterResetBuffer = builder.CreateResource("shadow_counter_reset_buffer", counterResetBuffer, &value, sizeof(std::uint32_t));

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t lightsNum = lightEntities.size();
        size_t meshesNum = meshes.size();

        dx12::ResourceDescription aabbBufferDescription;
        {
            aabbBufferDescription.SetSize({ (AlignToUAVCounterOffset(MAX_INSTANCES_NUM * sizeof(scene::AABBVolume))), 1 });
            aabbBufferDescription.SetStride(sizeof(scene::AABBVolume));
            aabbBufferDescription.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
        }
        _data.AABBBuffer = builder.CreateResource("aabb_buffer", aabbBufferDescription);

        std::uint32_t commandSize = static_cast<std::uint32_t>(sizeof(IndirectCommand));
        std::uint32_t alignedBufferSize = AlignToUAVCounterOffset(MAX_INSTANCES_NUM * commandSize);
        std::uint32_t counterSize = static_cast<std::uint32_t>(sizeof(UINT));

        dx12::ResourceDescription candidateBufferDescription;
        {
            candidateBufferDescription.SetSize({ alignedBufferSize, 1 });
            candidateBufferDescription.SetStride(commandSize);
            candidateBufferDescription.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
        }
        _data.CandidateInstancesBuffer.resize(lightsNum);
        for (size_t i = 0; i < lightsNum; ++i)
        {
            _data.CandidateInstancesBuffer[i] = builder.CreateResource(std::format("shadow_candidate_instances_buffer_{}", i), candidateBufferDescription);
        }

        dx12::ResourceDescription commandsBufferDescription;
        {
            commandsBufferDescription.SetSize({ alignedBufferSize + counterSize, 1 });
            commandsBufferDescription.SetStride(commandSize);
            commandsBufferDescription.SetUAVCounterOffset(alignedBufferSize);
            commandsBufferDescription.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
        }
        // TODO: optimize to use addresses not the full copy
        for (size_t frame = 0; frame < dx12::BACK_BUFFER_COUNT; ++frame)
        {
            _data.LightCommandBuffers[frame].resize(lightsNum);
            for (size_t i = 0; i < lightsNum; ++i)
            {
                _data.LightCommandBuffers[frame][i] = builder.CreateResource(std::format("shadow_culled_instances_buffer_{} (frame {})", i, frame), commandsBufferDescription);
            }
		}
    }

    void ShadowCullPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Shadow pass command list - culling");

        std::shared_ptr<dx12::Resource> frameBuffer = context.GetFrame()->_frameBuffer;
        std::shared_ptr<dx12::Resource> aabbBuffer = context.GetResourceNew(_data.AABBBuffer);
        std::shared_ptr<dx12::Resource> counterResetBuffer = context.GetResourceNew(_data.CounterResetBuffer);

        DescriptorHandle aabbBufferHandle = context.GetStaticResourceHandle(aabbBuffer->GetAsSRV());

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Shadow Pass | Culling");

        // Setup pipeline
        context.BindBindlessTable(commandList);
        commandList.SetPipelineState(_cullShadowsPipeline);

        for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

            std::shared_ptr<scene::Light> light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");

            std::shared_ptr<dx12::Resource> candidateInstancesBuffer = context.GetResourceNew(_data.CandidateInstancesBuffer[lightIndex]);
            std::shared_ptr<dx12::Resource> outputCommandBuffer = context.GetResourceNew(_data.LightCommandBuffers[context.GetFrameIndex()][lightIndex]);

            DescriptorHandle inputCommandsHandle = context.GetStaticResourceHandle(candidateInstancesBuffer->GetAsSRV());
			DescriptorHandle outputCommandBufferHandle = context.GetStaticResourceHandle(outputCommandBuffer->GetAsUAV());

            IndirectCommand* commandsList = candidateInstancesBuffer->Map<IndirectCommand>();

            // Record all draw command to the commandBuffer

            for (size_t j = 0; j < objectsNum; ++j)
            {
                std::shared_ptr<scene::Mesh> mesh = meshes[j]->GetComponentAs<scene::Mesh>("Mesh");

                if (!mesh)
                {
                    continue;
                }

                IndirectCommand& command = commandsList[j];

                command.VertexBufferAddress = mesh->VertexBufferView.BufferLocation;
                command.VertexBufferSize = mesh->VertexBufferView.SizeInBytes;
                command.VertexBufferStride = mesh->VertexBufferView.StrideInBytes;

                if (!mesh->SkinningVertexData.empty())
                {
                    command.SkinBufferAddress = mesh->SkinningVertexBufferView.BufferLocation;
                    command.SkinBufferSize = mesh->SkinningVertexBufferView.SizeInBytes;
                    command.SkinBufferStride = mesh->SkinningVertexBufferView.StrideInBytes;
                }
				else // put any dummy data to the skinning buffer to avoid GPU errors
                {
                    command.SkinBufferAddress = mesh->VertexBufferView.BufferLocation;
                    command.SkinBufferSize = mesh->VertexBufferView.SizeInBytes;
                    command.SkinBufferStride = mesh->VertexBufferView.StrideInBytes;
                }

				//command.IndexBufferAddress = mesh->IndexBufferView.BufferLocation;
    //            command.IndexBufferSize = mesh->IndexBufferView.SizeInBytes;
				//command.IndexBufferFormat = mesh->IndexBufferView.Format;

				command.FrameBufferAddress = frameBuffer->OffsetGPU();
				command.InstanceIndex = static_cast<std::uint32_t>(j);
				command.LightIndex = static_cast<std::uint32_t>(lightIndex);

                command.DrawArguments.VertexCountPerInstance = mesh->VertexData.size();
                command.DrawArguments.InstanceCount = 1;
                command.DrawArguments.StartVertexLocation = 0;
                command.DrawArguments.StartInstanceLocation = 0;
            }

            // Write all models bounding boxes to the buffer
            for (size_t j = 0, count = 0; j < objectsNum; ++j)
            {
                if (std::shared_ptr<scene::Mesh> mesh = _scene->GetRootNodes()[j]->GetComponentAs<scene::Mesh>("Mesh"))
                {
                    DirectX::XMVECTOR* data = aabbBuffer->Map<DirectX::XMVECTOR>();
                    scene::AABBVolume aabb = mesh->GlobalAABB;

                    data[count++] = aabb.Min;
                    data[count++] = aabb.Max;
                }
            }

            // Transition resources
            std::vector<dx12::ResourceBarrier> barriers =
            {
                { outputCommandBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,    D3D12_RESOURCE_STATE_COPY_DEST }
            };
            commandList.TransitionBarriers(barriers);

            // Reset commands counter
            std::uint32_t counterBufferOffset = outputCommandBuffer->GetResourceDescription().GetSize().x - sizeof(UINT);
            commandList.CopyBufferRegion(*counterResetBuffer, *outputCommandBuffer, sizeof(UINT), 0, counterBufferOffset);

            // Transition resources
            barriers =
            {
                { outputCommandBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
            };
            commandList.TransitionBarriers(barriers);

            PassConstants passConstants =
            {
				.LightIndex = lightIndex,
                .CommandsCount = static_cast<std::uint32_t>(objectsNum),
                .AABBBufferIndex = aabbBufferHandle.Index,
                .InputCommandsBufferIndex = inputCommandsHandle.Index,
                .OutputCommandsBufferIndex = outputCommandBufferHandle.Index
            };

            // Setup root signature
			commandList.SetCBV(0, context.GetFrame()->_frameBuffer->OffsetGPU());
            commandList.SetConstants(1, 5, &passConstants);

            // Dispatch culling compute shader
            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(objectsNum / (float)CULLING_PASS_THREADS_NUM);
            commandList.Dispatch(xThreadGroups);

            PIXEndEvent(commandList.GetDXCommandList().Get());
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
