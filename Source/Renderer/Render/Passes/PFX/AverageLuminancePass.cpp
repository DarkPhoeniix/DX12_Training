#include "RendererPCH.h"

#include "AverageLuminancePass.h"

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

    AverageLuminancePass::AverageLuminancePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<AverageLuminancePassData>("Average Luminance Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _averageLuminancePipeline.Parse("PipelineDescriptions\\AverageLuminancePipeline.tech");

        dx12::ResourceDescription lumDesc;
        {
            lumDesc.SetSize({ sizeof(float), 1 });
            lumDesc.SetStride(sizeof(float));
            lumDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
        }
        _prevLuminance.CreateCommitedResource(lumDesc);
    }

    void AverageLuminancePass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.LuminanceHistogram = builder.ReadResource(LUM_HISTOGRAM);

        dx12::ResourceDescription lumDesc;
        {
            lumDesc.SetSize({ sizeof(float), 1 });
            lumDesc.SetStride(sizeof(float));
            lumDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
        }
        _data.AverageLuminance = builder.CreateResource(AVERAGE_LUM, lumDesc);
    }

    void AverageLuminancePass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Luminance histogram pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "Luminance histogram");
        {
            // Copy and setup needed resources

            std::shared_ptr<dx12::Resource> luminanceHistogram = context.GetResource(_data.LuminanceHistogram);
            std::shared_ptr<dx12::Resource> averageLuminance = context.GetResource(_data.AverageLuminance);

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { &_prevLuminance,           D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_COPY_DEST },
                { averageLuminance.get(),   D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_COPY_SOURCE }
            };
            commandList.TransitionBarriers(barriers);

            commandList.CopyResource(*averageLuminance, _prevLuminance);
            
            barriers =
            {
                { &_prevLuminance,           D3D12_RESOURCE_STATE_COPY_DEST,     D3D12_RESOURCE_STATE_COMMON },
                { averageLuminance.get(),   D3D12_RESOURCE_STATE_COPY_SOURCE,   D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);

            // Setup pipeline state

            commandList.SetPipelineState(_averageLuminancePipeline);

            // Setup root signature components

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            std::uint32_t size = viewportSize.x * viewportSize.y;

            float adaptationSpeed = std::min((_scene->GetCache().GetDeltaTime() * 2.5f), 1.0f);

            commandList.SetConstants(0, 1, &size);
            commandList.SetConstants(0, 1, &MIN_LOG_LUM, 1);
            commandList.SetConstants(0, 1, &LOG_LUM_RANGE, 2);
            commandList.SetConstants(0, 1, &adaptationSpeed, 3);
            commandList.SetSRV(1, _prevLuminance.OffsetGPU());
            commandList.SetUAV(2, luminanceHistogram->OffsetGPU());
            commandList.SetUAV(3, averageLuminance->OffsetGPU());

            // Execute

            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(LUM_HISTOGRAM_THREADS_NUM));
            std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(LUM_HISTOGRAM_THREADS_NUM));
            commandList.Dispatch();
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
