#include "RendererPCH.h"

#include "GUIPass.h"

#include "ResourceTable.h"

#include "Editor.h"
#include "Scene/Entity/Components/Camera.h"

#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/RenderHelpers.h"
#include "Utility/DebugInfo.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

#include "Render/Passes/PassResources.h"

namespace render
{
    GUIPass::GUIPass(scene::Scene* scene, scene::Camera* camera)
        : RenderPass<GUIPassData>("GUI Pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
        gui::Editor::SetScene(_scene); // TODO: remove 
    }

    void GUIPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.Target = builder.WriteResource(TARGET);
        _data.Depth = builder.ReadResource(DEPTH);
    }

    void GUIPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Render GUI command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "GUI");
        {
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

            D3D12_CPU_DESCRIPTOR_HANDLE rtv = context.GetCPUHandle(target->GetAsRTV());
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = context.GetCPUHandle(depth->GetAsDSV());

            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList.TransitionBarrier(*depth, D3D12_RESOURCE_STATE_DEPTH_WRITE);

            commandList.SetViewport(_camera->GetViewport());
            commandList.SetRenderTarget(&rtv, &dsv);

            gui::Editor::NewFrame();

            gui::Editor::Update();
            gui::Editor::Render(commandList);

            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*depth, D3D12_RESOURCE_STATE_COMMON);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
