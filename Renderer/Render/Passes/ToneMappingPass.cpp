#include "RendererPCH.h"

#include "ToneMappingPass.h"

#include "Scene/Entity/Components/Camera.h"

namespace render
{
    void ToneMappingPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "ToneMappingPass";

        _lumHistogramPipeline.Parse("PipelineDescriptions\\BuildLuminanceHistogramPipeline.tech");
        _avglumHistogramPipeline.Parse("PipelineDescriptions\\AverageLuminancePipeline.tech");
        _lumDownscale1Pipeline.Parse("PipelineDescriptions\\LuminanceDownscale1PassPipeline.tech");
        _lumDownscale2Pipeline.Parse("PipelineDescriptions\\LuminanceDownscale2PassPipeline.tech");
        _toneMappingPipeline.Parse("PipelineDescriptions\\ToneMappingPipeline.tech");

        {
            dx12::ResourceDescription desc;
            DirectX::XMUINT2 size = _activeCamera->GetViewport().GetSize();
            desc.SetSize({ (size.x * size.y) * 4 / (16 * 1024), 1 });
            desc.SetResourceType(dx12::EResourceType::Buffer | dx12::EResourceType::Unordered);

            _averageLuminance.CreateCommitedResource(desc);
        }
    }

    void ToneMappingPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void ToneMappingPass::Execute()
    {
        BuildLuminanceHistogram();
        AvgLuminance();
        Tonemapping();
    }

    void ToneMappingPass::BuildLuminanceHistogram()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_lumDownscale1Pipeline);
        task->SetName("lum_histogram");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Luminance histogram pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "Luminance histogram");
        {
            commandList.SetPipelineState(_lumHistogramPipeline);

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::Resource* target = sceneTable.GetResourceByName("HDR_Lightpass", dx12::ResourceViewType::SRV);

            frameTable.CopyDescriptor(target, dx12::ResourceViewType::SRV, sceneTable);

            _frame->BindDescriptorHeaps(commandList);

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            //DirectX::XMUINT2 downscaledTexSize = { viewportSize.x / 4, viewportSize.y / 4 };
            //std::uint32_t domainSize = downscaledTexSize.x * downscaledTexSize.y;
            //std::uint32_t xThreadGroups = (uint32_t)std::ceilf((viewportSize.x * viewportSize.y) / float(16 * 1024));

            constexpr auto minLogLuminance = -10.0f;
            constexpr auto maxLogLuminance = 2.0f;
            constexpr auto logLuminanceRange = 1.0f / (maxLogLuminance - minLogLuminance);

            CacheGPU::DataHandle histogram = _frame->GetCache().RequestPlacement("lumHist", 256 * sizeof(std::uint32_t));

            commandList.SetConstants(0, 1, &viewportSize.x);
            commandList.SetConstants(0, 1, &viewportSize.y, 1);
            commandList.SetConstants(0, 1, &minLogLuminance, 2);
            commandList.SetConstants(0, 1, &logLuminanceRange, 3);
            commandList.SetDescriptorTable(1, frameTable.GetResourceGPUHandle(target, dx12::ResourceViewType::SRV));
            commandList.SetUAV(2, histogram.DataGPU);

            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);
            commandList.Dispatch(xThreadGroups, yThreadGroups);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    void ToneMappingPass::AvgLuminance()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_lumDownscale1Pipeline);
        task->SetName("avg_lum");
        task->AddDependency("lum_histogram");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Average luminance pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "Average luminance");
        {
            commandList.SetPipelineState(_avglumHistogramPipeline);

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::Resource* target = sceneTable.GetResourceByName("HDR_Lightpass", dx12::ResourceViewType::SRV);

            frameTable.CopyDescriptor(target, dx12::ResourceViewType::SRV, sceneTable);

            _frame->BindDescriptorHeaps(commandList);

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();

            constexpr auto minLogLuminance = -10.0f;
            constexpr auto maxLogLuminance = 4.0f;
            constexpr auto logLuminanceRange = (maxLogLuminance - minLogLuminance);

            CacheGPU::DataHandle histogram = _frame->GetCache().GetResourcePlacement("lumHist");
            CacheGPU::DataHandle avgLum = _frame->GetCache().RequestPlacement("avgLumHist", sizeof(std::uint32_t));
            CacheGPU::DataHandle prevAvgLuminance = _frame->GetCache().RequestPlacement("prevAvgLum1", sizeof(std::uint32_t));
            CacheGPU::DataHandle handle = _frame->Prev->GetCache().GetResourcePlacement("avgLumHist");
            float* lumData = (float*)prevAvgLuminance.DataCPU;
            if (handle.DataCPU)
            {
                lumData[0] = *((float*)handle.DataCPU);
                _adaptation = std::min((_scene->GetCache().GetDeltaTime() * 2.5f), 1.0f);
            }
            else
            {
                lumData[0] = 0.0001f;
                _adaptation = 0.0f;
            }

            std::uint32_t size = viewportSize.x * viewportSize.y;
            float delta = _scene->GetCache().GetDeltaTime();
            float tau = 1.1f;

            commandList.SetConstants(0, 1, &size);
            commandList.SetConstants(0, 1, &minLogLuminance, 1);
            commandList.SetConstants(0, 1, &logLuminanceRange, 2);
            commandList.SetConstants(0, 1, &delta, 3);
            commandList.SetConstants(0, 1, &_adaptation, 4);
            commandList.SetSRV(1, prevAvgLuminance.DataGPU);
            commandList.SetUAV(2, histogram.DataGPU);
            commandList.SetUAV(3, avgLum.DataGPU);

            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);
            commandList.Dispatch();
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    void ToneMappingPass::Downscale1()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_lumDownscale1Pipeline);
        task->SetName("downscale1");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Downscale 1 pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 6, "Luminance downscale - pass 1");
        {
            commandList.SetPipelineState(_lumDownscale1Pipeline);

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::Resource* target = sceneTable.GetResourceByName("HDR_Lightpass", dx12::ResourceViewType::SRV);

            frameTable.CopyDescriptor(target, dx12::ResourceViewType::SRV, sceneTable);

            _frame->BindDescriptorHeaps(commandList);

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            DirectX::XMUINT2 downscaledTexSize = { viewportSize.x / 4, viewportSize.y / 4 };
            std::uint32_t domainSize = downscaledTexSize.x * downscaledTexSize.y;
            std::uint32_t xThreadGroups = (uint32_t)std::ceilf((viewportSize.x * viewportSize.y) / float(16 * 1024));

            CacheGPU::DataHandle avgLuminance = _frame->GetCache().RequestPlacement("avgLum", xThreadGroups);

            commandList.SetConstants(0, 1, &downscaledTexSize.x);
            commandList.SetConstants(0, 1, &downscaledTexSize.y, 1);
            commandList.SetConstants(0, 1, &domainSize, 2);
            commandList.SetConstants(0, 1, &xThreadGroups, 3);
            commandList.SetDescriptorTable(1, frameTable.GetResourceGPUHandle(target, dx12::ResourceViewType::SRV));
            commandList.SetUAV(2, avgLuminance.DataGPU);

            commandList.Dispatch(xThreadGroups);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    void ToneMappingPass::Downscale2()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_lumDownscale2Pipeline);
        task->SetName("downscale2");
        task->AddDependency("downscale1");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Downscale 2 pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 6, "Luminance downscale - pass 2");
        {
            commandList.SetPipelineState(_lumDownscale2Pipeline);

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::Resource* target = frameTable.GetResourceByName("HDR_Lightpass", dx12::ResourceViewType::SRV);

            _frame->BindDescriptorHeaps(commandList);

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            DirectX::XMUINT2 downscaledTexSize = { viewportSize.x / 4, viewportSize.y / 4 };
            std::uint32_t domainSize = downscaledTexSize.x * downscaledTexSize.y;
            std::uint32_t xThreadGroups = (uint32_t)std::ceilf((viewportSize.x * viewportSize.y) / float(16 * 1024));

            CacheGPU::DataHandle avgLuminance = _frame->GetCache().GetResourcePlacement("avgLum");
            CacheGPU::DataHandle avgLuminanceFinal = _frame->GetCache().RequestPlacement("avgLumFinal", 4);
            CacheGPU::DataHandle prevAvgLuminance = _frame->GetCache().RequestPlacement("prevAvgLum", 4);
            CacheGPU::DataHandle handle = _frame->Prev->GetCache().GetResourcePlacement("avgLumFinal");
            float* lumData = (float*)prevAvgLuminance.DataCPU;
            if (handle.DataCPU)
            {
                lumData[0] = *((float*)handle.DataCPU);
                _adaptation = std::min((_scene->GetCache().GetDeltaTime() * 2.5f), 1.0f);
            }
            else
            {
                lumData[0] = 0.0001f;
                _adaptation = 0.0f;
            }

            commandList.SetConstants(0, 1, &downscaledTexSize.x);
            commandList.SetConstants(0, 1, &downscaledTexSize.y, 1);
            commandList.SetConstants(0, 1, &domainSize, 2);
            commandList.SetConstants(0, 1, &xThreadGroups, 3);
            commandList.SetConstants(0, 1, &_adaptation, 4);
            commandList.SetSRV(1, avgLuminance.DataGPU);
            commandList.SetSRV(2, prevAvgLuminance.DataGPU);
            commandList.SetUAV(3, avgLuminanceFinal.DataGPU);

            int groupNum = (viewportSize.x * viewportSize.y) / 64;
            commandList.Dispatch(groupNum);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    void ToneMappingPass::Tonemapping()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_toneMappingPipeline);
        task->SetName("tonemapping");
        task->AddDependency("avg_lum");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Tone mapping command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 7, "Tone Mapping");
        {
            commandList.SetPipelineState(_toneMappingPipeline);

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::Resource* hdr = frameTable.GetResourceByName("HDR_Lightpass", dx12::ResourceViewType::SRV);
            dx12::Resource* target = &_frame->GetTargetTexture();

            _frame->BindDescriptorHeaps(commandList);

            CacheGPU::DataHandle avgLuminance = _frame->GetCache().GetResourcePlacement("avgLumHist");

            float grey = 0.725f;
            float white = 5.5f;
            commandList.SetConstants(0, 1, &grey);
            commandList.SetConstants(0, 1, &white, 1);
            commandList.SetSRV(1, avgLuminance.DataGPU);
            commandList.SetDescriptorTable(2, frameTable.GetResourceGPUHandle(hdr, dx12::ResourceViewType::SRV));
            commandList.SetDescriptorTable(3, frameTable.GetResourceGPUHandle(target, dx12::ResourceViewType::UAV));

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
