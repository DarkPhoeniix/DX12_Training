#include "RendererPCH.h"

#include "SkyboxPass.h"

#include "Render/Helpers/RenderHelpers.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Skybox.h"

namespace render
{
    void SkyboxPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "SkyboxPass";

        _skyboxPipeline.Parse("PipelineDescriptions\\SkyboxPipeline.tech");
    }

    void SkyboxPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void SkyboxPass::Execute()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_skyboxPipeline);
        task->SetName("skybox");
        _tasks.push_back(task);

        std::shared_ptr<scene::Entity> entity = _scene->FindNodeByComponentName("Skybox");
        if (!entity)
        {
            return;
        }

        scene::Skybox* skybox = entity->GetComponentAs<scene::Skybox>("Skybox");

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Render skybox command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "Skybox");
        {
            commandList.SetPipelineState(_skyboxPipeline);

            Helpers::SetupSceneDataGPU(*_scene, commandList, &_frame->GetCache());

            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::ResourceTable& frameTable = _frame->GetResourceTable();

            dx12::Resource* target = &_frame->GetTargetTexture();
            dx12::Resource* skyboxTexture = skybox->SkydomeTexture.get();
            dx12::Resource* depth = &_gBuffer->GetDepthTexture();
            
            frameTable.CopyDescriptor(skyboxTexture, dx12::ResourceViewType::SRV, sceneTable);

            _frame->BindDescriptorHeaps(commandList);

            commandList.SetDescriptorTable(3, frameTable.GetResourceGPUHandle(depth, dx12::ResourceViewType::SRV));
            commandList.SetDescriptorTable(4, frameTable.GetResourceGPUHandle(skyboxTexture, dx12::ResourceViewType::SRV));
            commandList.SetDescriptorTable(5, frameTable.GetResourceGPUHandle(target, dx12::ResourceViewType::UAV));

            DirectX::XMUINT2 viewportSize = _activeCamera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
