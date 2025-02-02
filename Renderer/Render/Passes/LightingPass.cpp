#include "RendererPCH.h"

#include "LightingPass.h"

#include "ResourceTable.h"

#include "Scene/Entity/Components/Camera.h"
#include "Render/Helpers/RenderHelpers.h"

namespace render
{
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
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Lighting pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 4, "Deferred Shading");
        {
            commandList.SetPipelineState(_deferredPipeline);

            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            auto sceneTable = _scene->GetCache().GetTextureTable();
            dx12::ResourceTable& gBufferTable = _gBuffer->GetResourceTable();

            dx12::Resource* target = &_frame->GetTargetTexture();
            dx12::Resource* albedoMetalness = &_gBuffer->GetAlbedoMetalnessTexture();
            dx12::Resource* normalSpecular = &_gBuffer->GetNormalTexture();
            dx12::Resource* depth = &_gBuffer->GetDepthTexture();

            frameTable.CopyDescriptor(albedoMetalness, dx12::ResourceViewType::SRV, gBufferTable);
            frameTable.CopyDescriptor(normalSpecular, dx12::ResourceViewType::SRV, gBufferTable);
            frameTable.CopyDescriptor(depth, dx12::ResourceViewType::SRV, gBufferTable);

            _frame->BindDescriptorHeaps(commandList);

            helpers::SetupSceneDataGPU(*_scene, commandList, _frame);

            commandList.SetDescriptorTable(3, frameTable.GetResourceGPUHandle(depth, dx12::ResourceViewType::SRV));
            commandList.SetDescriptorTable(4, frameTable.GetResourceGPUHandle(albedoMetalness, dx12::ResourceViewType::SRV));
            commandList.SetDescriptorTable(5, frameTable.GetResourceGPUHandle(normalSpecular, dx12::ResourceViewType::SRV));
            commandList.SetDescriptorTable(6, frameTable.GetDescriptorHeap(dx12::ResourceViewType::SRV).GetHeapStartGPUHandle());
            commandList.SetDescriptorTable(7, frameTable.GetResourceGPUHandle(target, dx12::ResourceViewType::UAV));

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
