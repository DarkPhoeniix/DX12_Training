#include "RendererPCH.h"

#include "FXAAPass.h"

#include "Core/RenderSettings.h"
#include "Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

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
		: RenderPass<FXAAPassData>("fxaa_pass", rg::RenderPassType::Compute)
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
			rhi::IndirectArgumentDescription argsDesc =
			{
				.Type = rhi::IndirectArgumentType::Dispatch
			};

			D3D12_INDIRECT_ARGUMENT_DESC argsDesc;
			argsDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

            _cmdSignature.AddArgument(argsDesc);

			_cmdSignature.Create(sizeof(D3D12_DISPATCH_ARGUMENTS), nullptr);
		}

		{
			rhi::ResourceDescription counterResetBuffer;
			counterResetBuffer.SetSize({ sizeof(std::uint32_t) * 6, 1 });
			counterResetBuffer.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
			counterResetBuffer.SetResourceType(rhi::ResourceType::Buffer | rhi::ResourceType::Dynamic);

			_paramsReset = ResourceFactory::Create("fxaa_reset_buffer", counterResetBuffer);
			_paramsReset->CreateCommitedResource(rhi::ResourceState::CopySource);
      
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
		rhi::ResourceDescription workCountersDesc;
		{
			workCountersDesc.SetSize({ sizeof(std::uint32_t) * 2, 1 });
			workCountersDesc.SetStride(sizeof(std::uint32_t));
			workCountersDesc.SetResourceType(rhi::ResourceType::Buffer | rhi::ResourceType::Unordered);
		}
        builder.DeclareBuffer("fxaa_work_counter", workCountersDesc);

		rhi::ResourceDescription workQueueDesc;
		{
			DirectX::XMUINT2 size = _camera->GetViewport().GetSize();
			std::uint32_t bufferSize = (size.x * size.y) + 128;
			workQueueDesc.SetSize({ bufferSize, 1 });
			workQueueDesc.SetStride(sizeof(std::uint32_t));
			workQueueDesc.SetResourceType(rhi::ResourceType::Buffer | rhi::ResourceType::Unordered);
		}
		builder.DeclareBuffer("fxaa_work_queue", workQueueDesc);
        builder.DeclareBuffer("fxaa_color_queue", workQueueDesc);

		rhi::ResourceDescription lumaBufferDesc;
		{
			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = DXGI_FORMAT_R8_UNORM;
			clearValue.Color[0] = 0.0f;
			clearValue.Color[1] = 0.0f;
			clearValue.Color[2] = 0.0f;
			clearValue.Color[3] = 0.0f;

			DirectX::XMUINT2 size = _camera->GetViewport().GetSize();
			lumaBufferDesc.SetSize({ size.x, size.y });
			lumaBufferDesc.SetFormat(DXGI_FORMAT_R8_UNORM);
			lumaBufferDesc.SetResourceType(rhi::ResourceType::Texture | rhi::ResourceType::Unordered);
		}
        builder.DeclareTexture("luma_texture", lumaBufferDesc);

		rhi::ResourceDescription indirectArgsDesc;
		{
			DirectX::XMUINT2 size = _camera->GetViewport().GetSize();
			indirectArgsDesc.SetSize({ sizeof(D3D12_DISPATCH_ARGUMENTS) * 2, 1 });
			indirectArgsDesc.SetStride(sizeof(D3D12_DISPATCH_ARGUMENTS));
			indirectArgsDesc.SetResourceType(rhi::ResourceType::Buffer | rhi::ResourceType::Unordered);
		}
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

			commandList->TransitionBarrier({ indirectArgs, rhi::ResourceState::IndirectArgument, rhi::ResourceState::CopyDest });

			commandList->CopyBuffer(_paramsReset, indirectArgs);

			commandList->TransitionBarrier({ indirectArgs, rhi::ResourceState::CopyDest, rhi::ResourceState::IndirectArgument });

			for (int x = 0; x < 2; ++x)
			{
				for (int y = 0; y < 2; ++y)
				{
					// Pass 1 begin

					commandList->TransitionBarrier({ indirectArgs, rhi::ResourceState::IndirectArgument, rhi::ResourceState::UnorderedAccess });

					context.BindBindlessTable(commandList);
					commandList->SetPipelineState(_FXAA_Pass1_Pipeline.get());

					DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();

					float xRcpTextureSize = 1.0f / viewportSize.x;
					float yRcpTextureSize = 1.0f / viewportSize.y;
					std::uint32_t lastQueueIndex = (workQueue->GetResourceDescription().GetSize().x / sizeof(std::uint32_t)) - 1;
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
					commandList->SetConstants(1, 10, &pass1Constants);

					int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
					int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);
					commandList->Dispatch(xThreadGroups, yThreadGroups);

					// Pass 1 end

					commandList->UAVBarrier(workCounters);

					// Pass ResolveWork begin

					commandList->SetPipelineState(_FXAA_ResolveWork_Pipeline.get());

					PassResolveConstants passResolveConstants =
					{
						.LastQueueIndex = lastQueueIndex,
						.IndirectParamsBufferIndex = context.GetBindlessIndex(_data.IndirectParams, rhi::ResourceViewType::UAV),
						.WorkQueueBufferIndex = context.GetBindlessIndex(_data.WorkQueue, rhi::ResourceViewType::UAV),
						.WorkCountsBufferIndex = context.GetBindlessIndex(_data.WorkCounters, rhi::ResourceViewType::UAV)
					};
					commandList->SetConstants(1, 4, &passResolveConstants);

					commandList->Dispatch();

					// Pass ResolveWork end

					commandList->UAVBarrier(workCounters);

					// Pass 2 begin

					commandList->TransitionBarrier({ indirectArgs, rhi::ResourceState::UnorderedAccess, rhi::ResourceState::IndirectArgument });

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
					commandList->SetConstants(1, 9, &pass2Constants);

					commandList->SetPipelineState(_FXAA_Pass2H_Pipeline.get());
					commandList->ExecuteIndirect(_cmdSignature.get(), 1, indirectArgs, nullptr, 0);

					commandList->SetPipelineState(_FXAA_Pass2V_Pipeline.get());
					commandList->ExecuteIndirect(_cmdSignature.get(), 1, indirectArgs, nullptr, 12);

					// Pass 2 end

					commandList->UAVBarrier(target);
				}
			}
		}

		commandList->Close();
	}
}
