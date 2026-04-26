#include "RendererPCH.h"

#include "FXAAPass.h"

#include "Core/RenderSettings.h"
#include "Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

#include "RHI/ResourceBarrier.h"

namespace
{
	struct Pass1Constants
	{
		float ContrastThreshold;
		float SubpixelRemoval;
		DirectX::XMUINT2 StartPixel;
		std::uint32_t LastQueueIndex;

		std::uint32_t InputTextureIndex;
		std::uint32_t WorkCountBufferIndex;
		std::uint32_t WorkQueueBufferIndex;
		std::uint32_t ColorQueueBufferIndex;
		std::uint32_t LumaTextureIndex;
	};

	struct Pass2Constants
	{
		DirectX::XMFLOAT2 RcpTextureSize;
		DirectX::XMUINT2 StartPixel;
		std::uint32_t LastQueueIndex;

		std::uint32_t LumaTextureIndex;
		std::uint32_t WorkQueueBufferIndex;
		std::uint32_t ColorQueueBufferIndex;
		std::uint32_t OutputTextureIndex;
	};

	struct PassResolveConstants
	{
		std::uint32_t LastQueueIndex;

		std::uint32_t IndirectParamsBufferIndex;
		std::uint32_t WorkQueueBufferIndex;
		std::uint32_t WorkCountsBufferIndex;
	};

	struct Constants
	{
		float xRcpTextureSize;
		float yRcpTextureSize;
		float ContrastThreshold = 0.1f; // default = 0.2, lower is more expensive
		float SubpixelRemoval = 0.75f; // default = 0.75, lower blurs less
		std::uint32_t LastQueueIndex;
		std::uint32_t xStartPixel;
		std::uint32_t yStartPixel;
	};
}

namespace render
{
	FXAAPass::FXAAPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<FXAAPassData>(device, "fxaa_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_FXAA_Pass1_Pipeline = _device->CreatePipelineState("PipelineDescriptions\\FXAA_Pass1.tech");
		_FXAA_ResolveWork_Pipeline = _device->CreatePipelineState("PipelineDescriptions\\FXAA_ResolveWork.tech");
		if (RenderSettings::DebugFXAA())
		{
			_FXAA_Pass2H_Pipeline = _device->CreatePipelineState("PipelineDescriptions\\FXAA_Pass2H_Debug.tech");
			_FXAA_Pass2V_Pipeline = _device->CreatePipelineState("PipelineDescriptions\\FXAA_Pass2V_Debug.tech");
		}
		else
		{
			_FXAA_Pass2H_Pipeline = _device->CreatePipelineState("PipelineDescriptions\\FXAA_Pass2H.tech");
			_FXAA_Pass2V_Pipeline = _device->CreatePipelineState("PipelineDescriptions\\FXAA_Pass2V.tech");
		}

		{
			// https://microsoft.github.io/DirectX-Specs/d3d/IndirectDrawing.html#root-constants--vertex-buffers
			rhi::IndirectArgumentDescription argsDesc = { .Type = rhi::IndirectArgumentType::Dispatch };
			_cmdSignature = device->CreateCommandSignature({ argsDesc }, nullptr, "fxaa_command_signature");
		}

		{
			rhi::BufferDescription counterResetBuffer =
			{
				.Size = sizeof(std::uint32_t) * 6,
				.Usage = rhi::ResourceUsage::Upload
			};
			_paramsReset = device->CreateBuffer(counterResetBuffer, rhi::ResourceState::GenericRead, "fxaa_reset_buffer");
      
			std::uint32_t* val = _paramsReset->Map<std::uint32_t>();
			val[0] = 0;
			val[1] = 1;
			val[2] = 1;
			val[3] = 0;
			val[4] = 1;
			val[5] = 1;
		}
	}

	void FXAAPass::Setup(rg::RenderPassBuilder& builder)
	{
		rhi::BufferDescription workCountersDesc =
		{
			.Size = sizeof(std::uint32_t) * 2,
			.Stride = sizeof(std::uint32_t),
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess
		};
        builder.DeclareBuffer("fxaa_work_counter", workCountersDesc);

		DirectX::XMUINT2 size = _camera->GetSize();
		std::uint32_t bufferSize = (size.x * size.y) + 128;
		rhi::BufferDescription queueDesc =
		{
			.Size = bufferSize,
			.Stride = sizeof(std::uint32_t),
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess
		};
		builder.DeclareBuffer("fxaa_work_queue", queueDesc);
        builder.DeclareBuffer("fxaa_color_queue", queueDesc);

		rhi::TextureDescription lumaBufferDesc =
		{
			.Width = size.x,
			.Height = size.y,
			.ClearValue = {},
			.Format = rhi::Format::R8_UNORM,
			.Dimension = rhi::TextureDimension::Texture2D,
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess
		};
        builder.DeclareTexture("luma_texture", lumaBufferDesc);

		rhi::BufferDescription indirectArgsDesc =
		{
			.Size = sizeof(std::uint32_t) * 3 * 2, // sizeof(DISPATCH_ARGS) * 2
			.Stride = sizeof(std::uint32_t) * 3,
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess
		};
        builder.DeclareBuffer("fxaa_indirect_args", indirectArgsDesc);

        _data.Target = builder.WriteTexture("hdr_target");
        _data.LumaBuffer = builder.WriteTexture("luma_texture");
        _data.WorkCounters = builder.WriteBuffer("fxaa_work_counter");
        _data.WorkQueue = builder.WriteBuffer("fxaa_work_queue");
        _data.ColorQueue = builder.WriteBuffer("fxaa_color_queue");
		_data.IndirectParams = builder.IndirectArgBuffer("fxaa_indirect_args");
	}

	void FXAAPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "FXAA Compute Pass", 6);

			std::shared_ptr<rhi::Buffer> indirectArgs = context.GetBuffer(_data.IndirectParams);
			std::shared_ptr<rhi::Buffer> workQueue = context.GetBuffer(_data.WorkQueue);
			std::shared_ptr<rhi::Buffer> workCounters = context.GetBuffer(_data.WorkCounters);
			std::shared_ptr<rhi::Texture> target = context.GetTexture(_data.Target);

			rhi::BufferBarrier indirectArgsBarrier = { indirectArgs, rhi::ResourceState::IndirectArgument, rhi::ResourceState::CopyDest };
			commandList->TransitionBarriers({ indirectArgsBarrier });

			commandList->CopyBuffer(_paramsReset, indirectArgs);

			indirectArgsBarrier = { indirectArgs, rhi::ResourceState::CopyDest, rhi::ResourceState::IndirectArgument };
			commandList->TransitionBarriers({ indirectArgsBarrier });

			for (int x = 0; x < 2; ++x)
			{
				for (int y = 0; y < 2; ++y)
				{
					// Pass 1 begin

					indirectArgsBarrier = { indirectArgs, rhi::ResourceState::IndirectArgument, rhi::ResourceState::UnorderedAccess };
					commandList->TransitionBarriers({ indirectArgsBarrier });

					commandList->SetComputePipelineState(_FXAA_Pass1_Pipeline.get());

					DirectX::XMUINT2 viewportSize = _camera->GetSize();

					float xRcpTextureSize = 1.0f / viewportSize.x;
					float yRcpTextureSize = 1.0f / viewportSize.y;
					std::uint32_t lastQueueIndex = (workQueue->GetSize() / sizeof(std::uint32_t)) - 1;
					std::uint32_t xStartPixel = (uint32_t)std::ceilf(viewportSize.x / 2.0f) * x;
					std::uint32_t yStartPixel = (uint32_t)std::ceilf(viewportSize.y / 2.0f) * y;

					Pass1Constants pass1Constants =
					{
						.ContrastThreshold = 0.1f,
						.SubpixelRemoval = 0.75f,
						.StartPixel = { xStartPixel, yStartPixel },
						.LastQueueIndex = lastQueueIndex,
						.InputTextureIndex = context.GetBindlessIndex(_data.Target, rhi::ResourceViewType::SRV),
						.WorkCountBufferIndex = context.GetBindlessIndex(_data.WorkCounters, rhi::ResourceViewType::SRV),
						.WorkQueueBufferIndex = context.GetBindlessIndex(_data.WorkQueue, rhi::ResourceViewType::SRV),
						.ColorQueueBufferIndex = context.GetBindlessIndex(_data.ColorQueue, rhi::ResourceViewType::SRV),
						.LumaTextureIndex = context.GetBindlessIndex(_data.LumaBuffer, rhi::ResourceViewType::UAV)
					};
					commandList->SetComputeCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
					commandList->SetComputeConstants(1, 10, &pass1Constants);

					int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
					int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);
					commandList->Dispatch(xThreadGroups, yThreadGroups);

					// Pass 1 end

					commandList->UAVBarrier(workCounters);

					// Pass ResolveWork begin

					commandList->SetComputePipelineState(_FXAA_ResolveWork_Pipeline.get());

					PassResolveConstants passResolveConstants =
					{
						.LastQueueIndex = lastQueueIndex,
						.IndirectParamsBufferIndex = context.GetBindlessIndex(_data.IndirectParams, rhi::ResourceViewType::UAV),
						.WorkQueueBufferIndex = context.GetBindlessIndex(_data.WorkQueue, rhi::ResourceViewType::UAV),
						.WorkCountsBufferIndex = context.GetBindlessIndex(_data.WorkCounters, rhi::ResourceViewType::UAV)
					};
					commandList->SetComputeCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
					commandList->SetComputeConstants(1, 4, &passResolveConstants);

					commandList->Dispatch();

					// Pass ResolveWork end

					commandList->UAVBarrier(workCounters);

					// Pass 2 begin

					indirectArgsBarrier = { indirectArgs, rhi::ResourceState::UnorderedAccess, rhi::ResourceState::IndirectArgument };
					commandList->TransitionBarriers({ indirectArgsBarrier });

					Pass2Constants pass2Constants =
					{
						.RcpTextureSize = { xRcpTextureSize, yRcpTextureSize },
						.StartPixel = { xStartPixel, yStartPixel },
						.LastQueueIndex = lastQueueIndex,
						.LumaTextureIndex = context.GetBindlessIndex(_data.LumaBuffer, rhi::ResourceViewType::SRV),
						.WorkQueueBufferIndex = context.GetBindlessIndex(_data.WorkQueue, rhi::ResourceViewType::SRV),
						.ColorQueueBufferIndex = context.GetBindlessIndex(_data.ColorQueue, rhi::ResourceViewType::SRV),
						.OutputTextureIndex = context.GetBindlessIndex(_data.Target, rhi::ResourceViewType::UAV)
					};
					commandList->SetComputeCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
					commandList->SetComputeConstants(1, 9, &pass2Constants);

					commandList->SetComputePipelineState(_FXAA_Pass2H_Pipeline.get());
					commandList->ExecuteIndirect(_cmdSignature.get(), 1, indirectArgs, nullptr, 0);

					commandList->SetComputePipelineState(_FXAA_Pass2V_Pipeline.get());
					commandList->ExecuteIndirect(_cmdSignature.get(), 1, indirectArgs, nullptr, 12);

					// Pass 2 end

					commandList->UAVBarrier(target);
				}
			}
		}

		commandList->Close();
	}
}
