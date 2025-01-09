#include "RendererPCH.h"

#include "LightingPass.h"

#include "Scene/Entity/Components/Camera.h"
#include "Render/Helpers/RenderHelpers.h"

void LightingPass::Inititalize()
{
    IRenderPass::Inititalize();

    _name = "LightingPass";

    _deferredPipeline.Parse("PipelineDescriptions\\DeferredShading.tech");
}

void LightingPass::Destroy()
{
    IRenderPass::Destroy();
}

void LightingPass::Execute()
{
    TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_deferredPipeline);
    task->SetName("deferred");
    task->AddDependency("g-pass");

    dx12::CommandList& commandList = *task->GetCommandLists().front();
    commandList.SetName("Lighting pass command list");

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 4, "Deferred Shading");
    {
        commandList.SetPipelineState(_deferredPipeline);

        Helpers::SetupSceneDataGPU(*_scene, commandList, &_frame->GetCache());

        dx12::DescriptorHeap& buffersHeap = _frame->GetDescriptorHeap(dx12::DescriptorHeapType::CBV_SRV_UAV);

        dx12::Resource* target = &_frame->GetTargetTexture();
        dx12::Resource* albedoMetalness = &_gBuffer->GetAlbedoMetalnessTexture();
        dx12::Resource* normalSpecular = &_gBuffer->GetNormalTexture();
        dx12::Resource* depth = &_gBuffer->GetDepthTexture();

        dx12::Device::CreateShaderResourceView(albedoMetalness->GetAsSRV(), buffersHeap);
        dx12::Device::CreateShaderResourceView(normalSpecular->GetAsSRV(), buffersHeap);
        dx12::Device::CreateShaderResourceView(depth->GetAsSRV(), buffersHeap);
        dx12::Device::CreateUnorderedAccessView(target->GetAsUAV(), buffersHeap);

        _frame->BindDescriptorHeaps(commandList);

        commandList.SetDescriptorTable(3, buffersHeap.GetResourceGPUHandle(depth, dx12::ResourceViewType::SRV));
        commandList.SetDescriptorTable(4, buffersHeap.GetResourceGPUHandle(albedoMetalness, dx12::ResourceViewType::SRV));
        commandList.SetDescriptorTable(5, buffersHeap.GetResourceGPUHandle(normalSpecular, dx12::ResourceViewType::SRV));
        commandList.SetDescriptorTable(6, buffersHeap.GetResourceGPUHandle(target, dx12::ResourceViewType::UAV));

        DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
        int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
        int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

        commandList.GetDXCommandList()->Dispatch(xThreadGroups, yThreadGroups, 1);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}
