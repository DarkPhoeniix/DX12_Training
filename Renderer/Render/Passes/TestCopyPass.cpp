#include "RendererPCH.h"

#include "TestCopyPass.h"

#include "CommandList.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

#include "Render/Passes/PassResources.h"

namespace render
{
    TestCopyPass::TestCopyPass(scene::Scene* scene, scene::Camera* camera)
        : RenderPass<TestCopyPassData>("Test Copy", rg::RenderPassType::Copy)
        , _scene(scene)
        , _camera(camera)
    {
    }

    void TestCopyPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.AlbedoMetallic = builder.ReadResource(ALBEDO_METALLIC);
        _data.HDR = builder.ReadResource(HDR_TARGET);

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
            targetDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget);
        }
        _data.Target = builder.CreateResource("Target", targetDesc);
    }

    void TestCopyPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Test copy pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Test Copy Pass");
        {
            std::shared_ptr<dx12::Resource> albedoMetallic = context.GetResource(_data.HDR);
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);
            
            commandList.TransitionBarrier(*albedoMetallic, D3D12_RESOURCE_STATE_COPY_SOURCE);
            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_COPY_DEST);

            commandList.CopyResource(*albedoMetallic, *target);

            commandList.TransitionBarrier(*albedoMetallic, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_COMMON);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
}
