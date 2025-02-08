#include "RendererPCH.h"

#include "ToneMappingPass.h"

#include "Scene/Entity/Components/Camera.h"
#include "Render/Helpers/RenderHelpers.h"

namespace render
{
    void ToneMappingPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "ToneMappingPass";

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
        Downscale1();
        Downscale2();
        Tonemapping();
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
            std::uint32_t domainSize = (viewportSize.x * viewportSize.y) / 16;
            std::uint32_t xThreadGroups = domainSize / 1024;
            commandList.SetConstant(0, downscaledTexSize.x);
            commandList.SetConstant(0, downscaledTexSize.y, 1);
            commandList.SetConstant(0, domainSize, 2);
            commandList.SetConstant(0, xThreadGroups, 3);
            commandList.SetDescriptorTable(1, frameTable.GetResourceGPUHandle(target, dx12::ResourceViewType::SRV));
            commandList.SetUAV(2, _averageLuminance.OffsetGPU(0));

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
            commandList.SetPipelineState(_lumDownscale1Pipeline);

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::Resource* target = frameTable.GetResourceByName("HDR_Lightpass", dx12::ResourceViewType::SRV);

            _frame->BindDescriptorHeaps(commandList);

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            DirectX::XMUINT2 downscaledTexSize = { viewportSize.x / 4, viewportSize.y / 4 };
            std::uint32_t domainSize = (viewportSize.x * viewportSize.y) / 16;
            std::uint32_t xThreadGroups = domainSize / 1024;
            commandList.SetConstant(0, downscaledTexSize.x);
            commandList.SetConstant(0, downscaledTexSize.y, 1);
            commandList.SetConstant(0, domainSize, 2);
            commandList.SetConstant(0, xThreadGroups, 3);
            commandList.SetDescriptorTable(1, frameTable.GetResourceGPUHandle(target, dx12::ResourceViewType::SRV));
            commandList.SetUAV(2, _averageLuminance.OffsetGPU(0));

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
        task->AddDependency("downscale2");
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

            float grey = 0.5025f;
            float white = 2.5f;
            commandList.SetConstants(0, 1, &grey);
            commandList.SetConstants(0, 1, &white, 1);
            commandList.SetSRV(1, _averageLuminance.OffsetGPU(0));
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
