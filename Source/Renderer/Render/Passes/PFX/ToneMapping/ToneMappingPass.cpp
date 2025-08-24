#include "RendererPCH.h"

#include "ToneMappingPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

namespace render
{
    namespace
    {
        constexpr std::uint32_t LUM_HISTOGRAM_BINS_NUM = 256;
        constexpr std::uint32_t LUM_HISTOGRAM_THREADS_NUM = 16;
        constexpr std::uint32_t TONE_MAPPING_THREADS_NUM = 8;

        constexpr float MIN_LOG_LUM = -6.0f;
        constexpr float MAX_LOG_LUM = 4.0f;
        constexpr float LOG_LUM_RANGE = (MAX_LOG_LUM - MIN_LOG_LUM);
        constexpr float RCP_LOG_LUM_RANGE = 1.0f / LOG_LUM_RANGE;

        constexpr float WHITE = 3.5f;
        constexpr float MIDDLE_GREY = 0.18f;
        constexpr float GAMMA = 2.2f;

        struct PassCB
        {
            float MiddleGrey;
            float White;
            float Gamma;

            std::uint32_t HDRTextureIndex;
            std::uint32_t AverageLuminanceBufferIndex;
            std::uint32_t TargetTextureIndex;
        };
    } // namespace unnamed

    ToneMappingPass::ToneMappingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<ToneMappingPassData>("Tone Mapping Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _toneMappingPipeline.Parse("PipelineDescriptions\\ToneMappingPipeline.tech");
    }

    void ToneMappingPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.HDRTarget = builder.ReadResourceNew("hdr_target");
        _data.AverageLuminance = builder.ReadResourceNew("average_luminance");

        dx12::ResourceDescription targetDesc;
        {
            D3D12_CLEAR_VALUE clearValue;
            clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            clearValue.Color[0] = 0.0f;
            clearValue.Color[1] = 0.0f;
            clearValue.Color[2] = 0.0f;
            clearValue.Color[3] = 0.0f;

            targetDesc.SetSize(_camera->GetViewport().GetSize());
            targetDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            targetDesc.SetClearValue(clearValue);
            targetDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget | dx12::ResourceType::Unordered);
        }
        _data.Target = builder.CreateResourceNew("render_target", targetDesc);
    }

    void ToneMappingPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Tone mapping command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 7, "Tone Mapping");
        {
            // Copy and setup needed resources

            std::shared_ptr<dx12::Resource> hdrTarget       = context.GetResourceNew(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> avgLuminance    = context.GetResourceNew(_data.AverageLuminance);
            std::shared_ptr<dx12::Resource> target          = context.GetResourceNew(_data.Target);

            DescriptorHandle hdrTargetHandle                = context.GetStaticResourceHandle(hdrTarget->GetAsSRV());
            DescriptorHandle avgLuminanceHandle             = context.GetStaticResourceHandle(avgLuminance->GetAsSRV());
            DescriptorHandle targetHandle                   = context.GetStaticResourceHandle(target->GetAsUAV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { hdrTarget,      D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { avgLuminance,   D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { target,         D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
            };
            commandList.TransitionBarriers(barriers);

            // Setup pipeline state

            context.BindBindlessTable(commandList);

            commandList.SetPipelineState(_toneMappingPipeline);

            // Setup root signature components

            PassCB constants =
            {
                .MiddleGrey = MIDDLE_GREY,
                .White = WHITE,
                .Gamma = GAMMA,

                .HDRTextureIndex = hdrTargetHandle.Index,
                .AverageLuminanceBufferIndex = avgLuminanceHandle.Index,
                .TargetTextureIndex = targetHandle.Index
            };
            commandList.SetCBV(0, context.GetFrame()->_frameBuffer->OffsetGPU());
            commandList.SetConstants(1, 6, &constants);

            // Execute

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(TONE_MAPPING_THREADS_NUM));
            std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(TONE_MAPPING_THREADS_NUM));

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { hdrTarget,      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,   D3D12_RESOURCE_STATE_COMMON },
                { avgLuminance,   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,   D3D12_RESOURCE_STATE_COMMON },
                { target,         D3D12_RESOURCE_STATE_UNORDERED_ACCESS,            D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
