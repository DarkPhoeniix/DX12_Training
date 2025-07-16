#include "RendererPCH.h"

#include "LuminanceHistogramPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Scene/Entity/Components/Camera.h"
#include "Render/Passes/PassResources.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

namespace render
{
    namespace
    {
        constexpr std::uint32_t LUM_HISTOGRAM_BINS_NUM = 256;
        constexpr std::uint32_t LUM_HISTOGRAM_THREADS_NUM = 16;
        constexpr std::uint32_t TONE_MAPPING_THREADS_NUM = 8;

        constexpr float MIN_LOG_LUM = -10.0f;
        constexpr float MAX_LOG_LUM = 4.0f;
        constexpr float LOG_LUM_RANGE = (MAX_LOG_LUM - MIN_LOG_LUM);
        constexpr float RCP_LOG_LUM_RANGE = 1.0f / LOG_LUM_RANGE;

        constexpr float MIDDLE_GREY = 0.775f;
        constexpr float WHITE = 2.5f;
    } // namespace unnamed

    LuminanceHistogramPass::LuminanceHistogramPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<LuminanceHistogramPassData>("Luminance Histogram Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _luminanceHistogramPipeline.Parse("PipelineDescriptions\\BuildLuminanceHistogramPipeline.tech");
    }

    void LuminanceHistogramPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.HDRTarget = builder.ReadResource(HDR_TARGET);

        dx12::ResourceDescription lumDesc;
        {
            lumDesc.SetSize({ LUM_HISTOGRAM_BINS_NUM * sizeof(std::uint32_t), 1 });
            lumDesc.SetStride(sizeof(std::uint32_t));
            lumDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
        }
        _data.LuminanceHistogram = builder.CreateResource(LUM_HISTOGRAM, lumDesc);
    }

    void LuminanceHistogramPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Luminance histogram pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "Luminance histogram");
        {
            // Copy and setup needed resources

            std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> luminanceHistogram = context.GetResource(_data.LuminanceHistogram);

            D3D12_GPU_DESCRIPTOR_HANDLE targetHandle = context.GetGPUHandle(hdrTarget->GetAsSRV());

            // Setup pipeline state

            commandList.SetPipelineState(_luminanceHistogramPipeline);

            // Setup root signature components

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { hdrTarget,            D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE},
                { luminanceHistogram,   D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            commandList.SetConstants(0, 1, &viewportSize.x);
            commandList.SetConstants(0, 1, &viewportSize.y, 1);
            commandList.SetConstants(0, 1, &MIN_LOG_LUM, 2);
            commandList.SetConstants(0, 1, &RCP_LOG_LUM_RANGE, 3);
            commandList.SetDescriptorTable(1, targetHandle);
            commandList.SetUAV(2, luminanceHistogram->OffsetGPU());

            // Execute

            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(LUM_HISTOGRAM_THREADS_NUM));
            std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(LUM_HISTOGRAM_THREADS_NUM));
            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { hdrTarget,            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { luminanceHistogram,   D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
