#include "RendererPCH.h"

#include "ShadowClearPass.h"

#include "ResourceTable.h"

#include "Scene/Entity/Components/Camera.h"

#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Transformation.h"

#include "Render/Helpers/RenderHelpers.h"
#include "Utility/DebugInfo.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

#include "Render/Passes/PassResources.h"

namespace
{
    constexpr std::uint32_t CULLING_PASS_THREADS_NUM = 16;

    // Data structure to match the command signature used for ExecuteIndirect.
    struct IndirectCommand
    {
        D3D12_GPU_VIRTUAL_ADDRESS VertexBufferAddress;
        UINT VertexBufferSize;
        UINT VertexBufferStride;
        D3D12_GPU_VIRTUAL_ADDRESS SkinBufferAddress;
        UINT SkinBufferSize;
        UINT SkinBufferStride;
        D3D12_GPU_VIRTUAL_ADDRESS IndexBufferAddress;
        UINT IndexBufferSize;
        UINT IndexBufferFormat;
        D3D12_GPU_VIRTUAL_ADDRESS SceneBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS ModelBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS BonesBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS LightsBufferAddress;
        UINT LightIndex;

        D3D12_DRAW_ARGUMENTS DrawArguments;
    };

    std::uint32_t AlignToUAVCounterOffset(std::uint32_t size)
    {
        return Math::AlignUp(size, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
    }
} // namespace unnamed

namespace render
{
    ShadowClearPass::ShadowClearPass(scene::Scene* scene, scene::Camera* camera)
        : RenderPass<ShadowClearPassData>("Shadow Clear Pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
    }

    void ShadowClearPass::Setup(rg::RenderPassBuilder& builder)
    {
    }

    void ShadowClearPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Shadow pass command list - clear");

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Shadow Pass | Clear");
        for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            scene::Light* light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");

            if (std::shared_ptr<dx12::Resource> shadowMap = light->ShadowMap)
            {
                PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

                // Copy needed descriptors
                context.GetResourceTable().CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::DSV, *_scene->GetCache().GetTextureTable());
                context.GetResourceTable().CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::SRV, *_scene->GetCache().GetTextureTable());

                // Transition resources
                commandList.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);

                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = context.GetCPUHandle(shadowMap->GetAsDSV());
                commandList.ClearDSV(depthHandle, D3D12_CLEAR_FLAG_DEPTH);

                PIXEndEvent(commandList.GetDXCommandList().Get());
            }
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
