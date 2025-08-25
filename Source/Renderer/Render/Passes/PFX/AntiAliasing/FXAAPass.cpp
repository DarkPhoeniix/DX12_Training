#include "RendererPCH.h"

#include "FXAAPass.h"

#include "CommandList.h"

#include "Scene/Entity/Components/Camera.h"
#include "Render/RenderSettings.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

#include "ResourceBarrier.h"

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
    FXAAPass::FXAAPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<FXAAPassData>("FXAA Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _FXAA_Pass1_Pipeline.Parse("PipelineDescriptions\\FXAA_Pass1.tech");
        _FXAA_ResolveWork_Pipeline.Parse("PipelineDescriptions\\FXAA_ResolveWork.tech");
        if (RenderSettings::DebugFXAA())
        {
            _FXAA_Pass2H_Pipeline.Parse("PipelineDescriptions\\FXAA_Pass2H_Debug.tech");
            _FXAA_Pass2V_Pipeline.Parse("PipelineDescriptions\\FXAA_Pass2V_Debug.tech");
        }
        else
        {
            _FXAA_Pass2H_Pipeline.Parse("PipelineDescriptions\\FXAA_Pass2H.tech");
            _FXAA_Pass2V_Pipeline.Parse("PipelineDescriptions\\FXAA_Pass2V.tech");
        }

        {
            // https://microsoft.github.io/DirectX-Specs/d3d/IndirectDrawing.html#root-constants--vertex-buffers
            D3D12_INDIRECT_ARGUMENT_DESC argsDesc[1];
            argsDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

            D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
            commandSignatureDesc.pArgumentDescs = argsDesc;
            commandSignatureDesc.NumArgumentDescs = _countof(argsDesc);
            commandSignatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);

            dx12::Device::GetDXDevice()->CreateCommandSignature(&commandSignatureDesc, nullptr, IID_PPV_ARGS(&_cmdSignature));
        }

        {
            dx12::ResourceDescription counterResetBuffer;
            counterResetBuffer.SetSize({ sizeof(std::uint32_t) * 6, 1 });
            counterResetBuffer.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
            counterResetBuffer.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);

            _paramsReset = ResourceFactory::Create("FXAA reset buffer", counterResetBuffer);
            _paramsReset->CreateCommitedResource(D3D12_RESOURCE_STATE_COPY_SOURCE);
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
        _data.Target = builder.WriteResource("hdr_target");

        dx12::ResourceDescription workCountersDesc;
        {
            workCountersDesc.SetSize({ sizeof(std::uint32_t) * 2, 1 });
            workCountersDesc.SetStride(sizeof(std::uint32_t));
            workCountersDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
        }
        _data.WorkCounters = builder.CreateResource("fxaa_work_counter", workCountersDesc);

        dx12::ResourceDescription workQueueDesc;
        {
            DirectX::XMUINT2 size = _camera->GetViewport().GetSize();
            std::uint32_t bufferSize = (size.x * size.y) + 128;
            workQueueDesc.SetSize({ bufferSize, 1 });
            workQueueDesc.SetStride(sizeof(std::uint32_t));
            workQueueDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
        }
        _data.WorkQueue = builder.CreateResource("fxaa_work_queue", workQueueDesc);
        _data.ColorQueue = builder.CreateResource("fxaa_color_queue", workQueueDesc);

        dx12::ResourceDescription lumaBufferDesc;
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
            lumaBufferDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        _data.LumaBuffer = builder.CreateResource("luma_texture", lumaBufferDesc);

        dx12::ResourceDescription indirectArgsDesc;
        {
            DirectX::XMUINT2 size = _camera->GetViewport().GetSize();
            indirectArgsDesc.SetSize({ sizeof(D3D12_DISPATCH_ARGUMENTS) * 2, 1});
            indirectArgsDesc.SetStride(sizeof(D3D12_DISPATCH_ARGUMENTS));
            indirectArgsDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
        }
        _data.IndirectParams = builder.CreateResource("fxaa_indirect_args", indirectArgsDesc);
    }

    void FXAAPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("FXAA command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 6, "FXAA Pass");
        {
            std::shared_ptr<dx12::Resource> workCounters    = context.GetResourceNew(_data.WorkCounters);
            std::shared_ptr<dx12::Resource> workQueue       = context.GetResourceNew(_data.WorkQueue);
            std::shared_ptr<dx12::Resource> colorQueue      = context.GetResourceNew(_data.ColorQueue);
            std::shared_ptr<dx12::Resource> luma            = context.GetResourceNew(_data.LumaBuffer);
            std::shared_ptr<dx12::Resource> indirectArgs    = context.GetResourceNew(_data.IndirectParams);
            std::shared_ptr<dx12::Resource> target          = context.GetResourceNew(_data.Target);

            DescriptorHandle lumaHandleSRV      = context.GetStaticResourceHandle(luma->GetAsSRV());
            DescriptorHandle lumaHandleUAV      = context.GetStaticResourceHandle(luma->GetAsUAV());
            DescriptorHandle workQueueSRV       = context.GetStaticResourceHandle(workQueue->GetAsSRV());
            DescriptorHandle workQueueUAV       = context.GetStaticResourceHandle(workQueue->GetAsUAV());
            DescriptorHandle colorQueueSRV      = context.GetStaticResourceHandle(colorQueue->GetAsSRV());
            DescriptorHandle colorQueueUAV      = context.GetStaticResourceHandle(colorQueue->GetAsUAV());
            DescriptorHandle workCountersUAV    = context.GetStaticResourceHandle(workCounters->GetAsUAV());
            DescriptorHandle indirectArgsUAV    = context.GetStaticResourceHandle(indirectArgs->GetAsUAV());
            DescriptorHandle targetHandleSRV    = context.GetStaticResourceHandle(target->GetAsSRV());
            DescriptorHandle targetHandleUAV    = context.GetStaticResourceHandle(target->GetAsUAV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { indirectArgs,   D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_COPY_DEST }
            };
            commandList.TransitionBarriers(barriers);

            commandList.CopyResource(*_paramsReset, *indirectArgs);

            barriers =
            {
                { indirectArgs,   D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT },
                { workQueue,      D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { colorQueue,     D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { luma,           D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { target,         D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
            };
            commandList.TransitionBarriers(barriers);

            for (int x = 0; x < 2; ++x)
            {
                for (int y = 0; y < 2; ++y)
                {
                    // Pass 1 begin

                    barriers =
                    {
                        { indirectArgs,   D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,         D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                        { workCounters,   D3D12_RESOURCE_STATE_COMMON,                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                        { workQueue,      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                        { colorQueue,     D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                        { luma,           D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                        { target,         D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                    };
                    commandList.TransitionBarriers(barriers);

                    context.BindBindlessTable(commandList);
                    commandList.SetPipelineState(_FXAA_Pass1_Pipeline);

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
						.InputTextureIndex = targetHandleSRV.Index,
                        .WorkCountBufferIndex = workCountersUAV.Index,
                        .WorkQueueBufferIndex = workQueueUAV.Index,
                        .ColorQueueBufferIndex = colorQueueUAV.Index,
                        .LumaTextureIndex = lumaHandleUAV.Index
                    };
                    commandList.SetCBV(0, context.GetFrame()->_frameBuffer->OffsetGPU());
					commandList.SetConstants(1, 10, &pass1Constants);

                    int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
                    int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);
                    commandList.Dispatch(xThreadGroups, yThreadGroups);

                    // Pass 1 end

                    commandList.UAVBarrier(workCounters);

                    // Pass ResolveWork begin

                    context.BindBindlessTable(commandList);
                    commandList.SetPipelineState(_FXAA_ResolveWork_Pipeline);

                    PassResolveConstants passResolveConstants =
                    {
                        .LastQueueIndex = lastQueueIndex,
						.IndirectParamsBufferIndex = indirectArgsUAV.Index,
                        .WorkQueueBufferIndex = workQueueUAV.Index,
						.WorkCountsBufferIndex = workCountersUAV.Index
                    };
                    commandList.SetCBV(0, context.GetFrame()->_frameBuffer->OffsetGPU());
                    commandList.SetConstants(1, 4, &passResolveConstants);

                    commandList.Dispatch();

                    // Pass ResolveWork end

                    commandList.UAVBarrier(workCounters);

                    // Pass 2 begin

                    barriers =
                    {
                        { indirectArgs,   D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT },
                        { workCounters,   D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_COMMON },
                        { workQueue,      D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                        { colorQueue,     D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                        { luma,           D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                        { target,         D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                    };
                    commandList.TransitionBarriers(barriers);

                    Pass2Constants pass2Constants =
                    {
						.RcpTextureSize = { xRcpTextureSize, yRcpTextureSize },
                        .StartPixel = { xStartPixel, yStartPixel },
                        .LastQueueIndex = lastQueueIndex,
                        .LumaTextureIndex = lumaHandleSRV.Index,
                        .WorkQueueBufferIndex = workQueueSRV.Index,
                        .ColorQueueBufferIndex = colorQueueSRV.Index,
						.OutputTextureIndex = targetHandleUAV.Index
                    };
                    commandList.SetCBV(0, context.GetFrame()->_frameBuffer->OffsetGPU());
                    commandList.SetConstants(1, 9, &pass2Constants);

                    context.BindBindlessTable(commandList);
                    commandList.SetPipelineState(_FXAA_Pass2H_Pipeline);
                    commandList.ExecuteIndirect(_cmdSignature, 1, *indirectArgs, nullptr, 0);

                    context.BindBindlessTable(commandList);
                    commandList.SetPipelineState(_FXAA_Pass2V_Pipeline);
                    commandList.ExecuteIndirect(_cmdSignature, 1, *indirectArgs, nullptr, 12);

                    // Pass 2 end

                    commandList.UAVBarrier(target);
                }
            }

            barriers =
            {
                { indirectArgs,   D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,         D3D12_RESOURCE_STATE_COMMON },
                { workQueue,      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { colorQueue,     D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { luma,           D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { target,         D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_COMMON },
            };
            commandList.TransitionBarriers(barriers);

        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
}
