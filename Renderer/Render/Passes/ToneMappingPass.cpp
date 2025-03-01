#include "RendererPCH.h"

#include "ToneMappingPass.h"

#include "Scene/Entity/Components/Camera.h"

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

    void ToneMappingPass::Initialize()
    {
        IRenderPass::Initialize();

        _name = "ToneMappingPass";

        _luminanceHistogramPipeline.Parse("PipelineDescriptions\\BuildLuminanceHistogramPipeline.tech");
        _averageLuminanceHistogramPipeline.Parse("PipelineDescriptions\\AverageLuminancePipeline.tech");
        _toneMappingPipeline.Parse("PipelineDescriptions\\ToneMappingPipeline.tech");

        {
            dx12::ResourceDescription resDesc;
            resDesc.SetSize({ sizeof(float), 1 });
            resDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);

            _averageFrameLum[0].CreateCommitedResource(resDesc);
            _averageFrameLum[1].CreateCommitedResource(resDesc);
            _averageFrameLum[2].CreateCommitedResource(resDesc);
        }
    }

    void ToneMappingPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void ToneMappingPass::Execute()
    {
        BuildLuminanceHistogram();
        CalculateAverageLuminance();
        ApplyToneMapping();
    }

    void ToneMappingPass::BuildLuminanceHistogram()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_luminanceHistogramPipeline);
        task->SetName("luminance_histogram");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Luminance histogram pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "Luminance histogram");
        {
            // Setup pipeline state

            commandList.SetPipelineState(_luminanceHistogramPipeline);

            // Copy and setup needed resources

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::Resource* target = sceneTable.GetResourceByName("HDR_Lightpass", dx12::ResourceViewType::SRV);

            frameTable.CopyDescriptor(target, dx12::ResourceViewType::SRV, sceneTable);

            _frame->BindDescriptorHeaps(commandList);

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            CacheGPU::DataHandle histogram = _frame->GetCache().RequestPlacement("luminanceHistogram", LUM_HISTOGRAM_BINS_NUM * sizeof(std::uint32_t));
            memset(histogram.DataCPU, 0, LUM_HISTOGRAM_BINS_NUM * sizeof(std::uint32_t));

            // Setup root signature components

            commandList.SetConstants(0, 1, &viewportSize.x);
            commandList.SetConstants(0, 1, &viewportSize.y, 1);
            commandList.SetConstants(0, 1, &MIN_LOG_LUM, 2);
            commandList.SetConstants(0, 1, &RCP_LOG_LUM_RANGE, 3);
            commandList.SetDescriptorTable(1, frameTable.GetResourceGPUHandle(target, dx12::ResourceViewType::SRV));
            commandList.SetUAV(2, histogram.DataGPU);

            // Execute

            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(LUM_HISTOGRAM_THREADS_NUM));
            std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(LUM_HISTOGRAM_THREADS_NUM));
            commandList.Dispatch(xThreadGroups, yThreadGroups);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    void ToneMappingPass::CalculateAverageLuminance()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_averageLuminanceHistogramPipeline);
        task->SetName("average_luminance");
        task->AddDependency("luminance_histogram");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Average luminance pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "Average luminance");
        {
            // Setup pipeline state

            commandList.SetPipelineState(_averageLuminanceHistogramPipeline);

            // Copy and setup needed resources

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::Resource* target = sceneTable.GetResourceByName("HDR_Lightpass", dx12::ResourceViewType::SRV);

            frameTable.CopyDescriptor(target, dx12::ResourceViewType::SRV, sceneTable);

            _frame->BindDescriptorHeaps(commandList);

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            std::uint32_t size = viewportSize.x * viewportSize.y;

            CacheGPU::DataHandle histogram = _frame->GetCache().GetResourcePlacement("luminanceHistogram");

            _adaptationSpeed = std::min((_scene->GetCache().GetDeltaTime() * 2.5f), 1.0f);

            // Setup root signature components

            commandList.SetConstants(0, 1, &size);
            commandList.SetConstants(0, 1, &MIN_LOG_LUM, 1);
            commandList.SetConstants(0, 1, &LOG_LUM_RANGE, 2);
            commandList.SetConstants(0, 1, &_adaptationSpeed, 3);
            commandList.SetSRV(1, _averageFrameLum[_frame->Prev->Index].OffsetGPU());
            commandList.SetUAV(2, histogram.DataGPU);
            commandList.SetUAV(3, _averageFrameLum[_frame->Index].OffsetGPU());

            // Execute

            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(LUM_HISTOGRAM_THREADS_NUM));
            std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(LUM_HISTOGRAM_THREADS_NUM));
            commandList.Dispatch();
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    void ToneMappingPass::ApplyToneMapping()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_toneMappingPipeline);
        task->SetName("tonemapping");
        task->AddDependency("average_luminance");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Tone mapping command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 7, "Tone Mapping");
        {
            // Setup pipeline state

            commandList.SetPipelineState(_toneMappingPipeline);

            // Copy and setup needed resources

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::Resource* hdr = frameTable.GetResourceByName("HDR_Lightpass", dx12::ResourceViewType::SRV);
            dx12::Resource* target = &_frame->GetTargetTexture();

            _frame->BindDescriptorHeaps(commandList);

            // Setup root signature components

            commandList.SetConstants(0, 1, &MIDDLE_GREY);
            commandList.SetConstants(0, 1, &WHITE, 1);
            commandList.SetSRV(1, _averageFrameLum[_frame->Index].OffsetGPU());
            commandList.SetDescriptorTable(2, frameTable.GetResourceGPUHandle(hdr, dx12::ResourceViewType::SRV));
            commandList.SetDescriptorTable(3, frameTable.GetResourceGPUHandle(target, dx12::ResourceViewType::UAV));

            // Execute

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(TONE_MAPPING_THREADS_NUM));
            std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(TONE_MAPPING_THREADS_NUM));

            commandList.Dispatch(xThreadGroups, yThreadGroups);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render