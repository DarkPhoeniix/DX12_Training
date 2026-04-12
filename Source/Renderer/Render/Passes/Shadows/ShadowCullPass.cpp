#include "RendererPCH.h"

#include "ShadowCullPass.h"

#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Volumes/AABBVolume.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

#include "RHI/CommandSignature.h"
#include "RHI/ResourceBarrier.h"

namespace
{
	constexpr std::uint32_t CULLING_PASS_THREADS_NUM = 16;
	constexpr std::uint32_t MAX_INSTANCES_NUM = 1u << 20;

	// Data structure to match the command signature used for ExecuteIndirect.
	struct alignas(16) IndirectCommand
	{
		std::uint64_t VertexBufferAddress;
		std::uint32_t VertexBufferSize;
		std::uint32_t VertexBufferStride;
		std::uint64_t SkinBufferAddress;
		std::uint32_t SkinBufferSize;
		std::uint32_t SkinBufferStride;
		std::uint64_t IndexBufferAddress;
		std::uint32_t IndexBufferSize;
		std::uint32_t IndexBufferFormat;

		std::uint64_t FrameBufferAddress;
		std::uint32_t InstanceIndex;
		std::uint32_t LightIndex;

		std::uint32_t IndexCountPerInstance;
		std::uint32_t InstanceCount;
		std::uint32_t StartIndexLocation;
		std::int32_t BaseVertexLocation;
		std::uint32_t StartInstanceLocation;
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
		return Math::AlignUp(size, 4096); // D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT
	}
} // namespace unnamed

namespace render
{
	ShadowCullPass::ShadowCullPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<ShadowCullPassData>(device, "shadow_culling_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_cullShadowsPipeline = _device->CreatePipelineState("PipelineDescriptions\\LightCulling_PointLight.tech");
	}

	void ShadowCullPass::Setup(rg::RenderPassBuilder& builder)
	{
		rhi::BufferDescription counterResetBuffer =
		{
			.Size = sizeof(std::uint32_t),
			.Stride = sizeof(std::uint32_t),
			.Usage = rhi::ResourceUsage::Upload
		};
		std::uint32_t value = 0;
        builder.DeclareBuffer("shadow_counter_reset_buffer", counterResetBuffer, &value, sizeof(std::uint32_t));

		std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
		std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
		size_t lightsNum = lightEntities.size();
		size_t meshesNum = meshes.size();

		rhi::BufferDescription aabbBufferDescription =
		{
			.Size = AlignToUAVCounterOffset(MAX_INSTANCES_NUM * sizeof(scene::AABBVolume)),
			.Stride = sizeof(scene::AABBVolume),
			.Usage = rhi::ResourceUsage::Upload
		};
		builder.DeclareBuffer("aabb_buffer", aabbBufferDescription);

		std::uint32_t commandSize = static_cast<std::uint32_t>(sizeof(IndirectCommand));
		std::uint32_t alignedBufferSize = AlignToUAVCounterOffset(MAX_INSTANCES_NUM * commandSize);
		std::uint32_t counterSize = static_cast<std::uint32_t>(sizeof(UINT));

		rhi::BufferDescription candidateBufferDescription =
		{
			.Size = alignedBufferSize,
			.Stride = commandSize,
			.Usage = rhi::ResourceUsage::Upload
		};
		_data.CandidateInstancesBuffer.resize(lightsNum);
		for (size_t i = 0; i < lightsNum; ++i)
		{
            builder.DeclareBuffer(std::format("shadow_candidate_instances_buffer_{}", i), candidateBufferDescription);
			_data.CandidateInstancesBuffer[i] = builder.UploadBuffer(std::format("shadow_candidate_instances_buffer_{}", i));
		}

		rhi::BufferDescription commandsBufferDescription =
		{
			.Size = alignedBufferSize + counterSize,
			.Stride = commandSize,
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess,
			.UAVCounterOffset = alignedBufferSize
		};
			_data.LightCommandBuffers.resize(lightsNum);
			for (size_t i = 0; i < lightsNum; ++i)
			{
                builder.DeclareBuffer(std::format("shadow_culled_instances_buffer_{}", i), commandsBufferDescription);
				_data.LightCommandBuffers[i] = builder.WriteBuffer(std::format("shadow_culled_instances_buffer_{}", i));
			}

        _data.CounterResetBuffer = builder.CopySrcBuffer("shadow_counter_reset_buffer");
        _data.AABBBuffer = builder.UploadBuffer("aabb_buffer");
	}

	void ShadowCullPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
			GPU_SCOPED_EVENT(commandList, "Shadow Culling Pass", 1);

			std::shared_ptr<rhi::Buffer> aabbBuffer = context.GetBuffer(_data.AABBBuffer);
			std::shared_ptr<rhi::Buffer> counterResetBuffer = context.GetBuffer(_data.CounterResetBuffer);

			rhi::CPUDescriptor aabbBufferHandle = context.GetDescriptor((rg::RGBufferId)aabbBuffer->GetID(), rhi::ResourceViewType::SRV);

			std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
			std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
			size_t objectsNum = meshes.size();

			// Setup pipeline
			commandList->SetComputePipelineState(_cullShadowsPipeline.get());

			for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
			{
				GPU_SCOPED_EVENT(commandList, lightEntities[lightIndex]->GetName().c_str(), 1);

				std::shared_ptr<rhi::Buffer> candidateInstancesBuffer = context.GetBuffer(_data.CandidateInstancesBuffer[lightIndex]);
				std::shared_ptr<rhi::Buffer> outputCommandBuffer = context.GetBuffer(_data.LightCommandBuffers[lightIndex]);

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

					command.IndexBufferAddress = mesh->IndexBufferView.BufferLocation;
					command.IndexBufferSize = mesh->IndexBufferView.SizeInBytes;
					command.IndexBufferFormat = (std::uint32_t)mesh->IndexBufferView.Format;

					command.FrameBufferAddress = context.GetFrameBuffer()->GetVirtualAddress();
					command.InstanceIndex = static_cast<std::uint32_t>(j);
					command.LightIndex = static_cast<std::uint32_t>(lightIndex);

					command.IndexCountPerInstance = mesh->IndexData.size();
					command.InstanceCount = 1;
					command.StartIndexLocation = 0;
					command.BaseVertexLocation = 0;
					command.StartInstanceLocation = 0;
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
				rhi::BufferBarrier outputCommandBufferBarrier = { outputCommandBuffer, rhi::ResourceState::UnorderedAccess, rhi::ResourceState::CopyDest };
				commandList->TransitionBarriers({ outputCommandBufferBarrier });

				// Reset commands counter
				std::uint32_t counterBufferOffset = outputCommandBuffer->GetSize() - sizeof(std::uint32_t);
				commandList->CopyBufferRegion(counterResetBuffer, outputCommandBuffer, sizeof(std::uint32_t), 0, counterBufferOffset);

				// Transition resources
				outputCommandBufferBarrier = { outputCommandBuffer, rhi::ResourceState::CopyDest, rhi::ResourceState::UnorderedAccess };
				commandList->TransitionBarriers({ outputCommandBufferBarrier });

				PassConstants passConstants =
				{
					.LightIndex = lightIndex,
					.CommandsCount = static_cast<std::uint32_t>(objectsNum),
					.AABBBufferIndex = context.GetBindlessIndex(_data.AABBBuffer, rhi::ResourceViewType::SRV),
					.InputCommandsBufferIndex = context.GetBindlessIndex(_data.CandidateInstancesBuffer[lightIndex], rhi::ResourceViewType::SRV),
					.OutputCommandsBufferIndex = context.GetBindlessIndex(_data.LightCommandBuffers[lightIndex], rhi::ResourceViewType::UAV),
				};

				// Setup root signature
				commandList->SetComputeCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
				commandList->SetComputeConstants(1, 5, &passConstants);

				// Dispatch culling compute shader
				std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(objectsNum / (float)CULLING_PASS_THREADS_NUM);
				commandList->Dispatch(xThreadGroups);
			}
		}

		commandList->Close();
	}
} // namespace render
