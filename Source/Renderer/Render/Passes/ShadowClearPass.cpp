#include "RendererPCH.h"

#include "ShadowClearPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Scene/Entity/Components/Light.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

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
    ShadowClearPass::ShadowClearPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<ShadowClearPassData>("Shadow Clear Pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
    }

    void ShadowClearPass::Setup(rg::RenderPassBuilder& builder)
    {
        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::uint32_t lightsNum = lightEntities.size();

        _data.ShadowMaps.resize(lightsNum, rg::ResourceId(-1));
        for (size_t lightIndex = 0; lightIndex < lightsNum; ++lightIndex)
        {
            std::shared_ptr<scene::Entity> entity = lightEntities[lightIndex];
            std::shared_ptr<scene::Light> light = entity->GetComponentAs<scene::Light>("Light");

            if (light->CastShadows)
            {
                dx12::ResourceDescription shadowMapDesc;
                {
                    D3D12_CLEAR_VALUE clearValue;
                    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
                    clearValue.DepthStencil.Depth = 1;
                    clearValue.DepthStencil.Stencil = 0;

                    DirectX::XMUINT2 size = _camera->GetViewport().GetSize();
                    shadowMapDesc.SetSize({ size.x / 2, size.x / 2 });
                    shadowMapDesc.SetFormat(DXGI_FORMAT_D32_FLOAT);
                    shadowMapDesc.SetClearValue(clearValue);
                    switch (light->Type)
                    {
                    case scene::LightType::Spot:
                        shadowMapDesc.SetDepthOrArraySize(1);
                        break;
                    case scene::LightType::Point:
                        shadowMapDesc.SetDepthOrArraySize(6);
                        break;
                    }
                    shadowMapDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::DepthStencil);
                }
                _data.ShadowMaps[lightIndex] = builder.CreateResource(std::format("{}_ShadowMap", entity->GetName()), shadowMapDesc);
            }
        }
    }

    void ShadowClearPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Shadow pass command list - clear");

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Shadow Pass | Clear");
        for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            if (_data.ShadowMaps[lightIndex] == rg::ResourceId(-1))
            {
                continue;
            }

            PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

            std::shared_ptr<dx12::Resource> shadowMap = context.GetResource(_data.ShadowMaps[lightIndex]);

            // Transition resources
            std::vector<dx12::ResourceBarrier> barriers =
            {
                { shadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,    D3D12_RESOURCE_STATE_DEPTH_WRITE }
            };
            commandList.TransitionBarriers(barriers);

            D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = context.GetCPUHandle(shadowMap->GetAsDSV());
            commandList.ClearDSV(depthHandle, D3D12_CLEAR_FLAG_DEPTH);

            PIXEndEvent(commandList.GetDXCommandList().Get());
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
