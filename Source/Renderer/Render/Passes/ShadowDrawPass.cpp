#include "RendererPCH.h"

#include "ShadowDrawPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Render/Helpers/RenderHelpers.h"
#include "Scene/Entity/Components/Camera.h"
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
    ShadowDrawPass::ShadowDrawPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<ShadowDrawPassData>("Shadow Draw Pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
        _spotLightShadowsPipeline.Parse("PipelineDescriptions\\Shadow_SpotLight.tech");
        _pointLightShadowsPipeline.Parse("PipelineDescriptions\\Shadow_PointLight.tech");

        {
            // https://microsoft.github.io/DirectX-Specs/d3d/IndirectDrawing.html#root-constants--vertex-buffers
            D3D12_INDIRECT_ARGUMENT_DESC argsDesc[9];
            argsDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
            argsDesc[0].VertexBuffer.Slot = 0;

            argsDesc[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
            argsDesc[1].VertexBuffer.Slot = 1;

            argsDesc[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;

            argsDesc[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
            argsDesc[3].ConstantBufferView.RootParameterIndex = 0;

            argsDesc[4].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
            argsDesc[4].ConstantBufferView.RootParameterIndex = 1;

            argsDesc[5].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
            argsDesc[5].ShaderResourceView.RootParameterIndex = 2;

            argsDesc[6].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
            argsDesc[6].ShaderResourceView.RootParameterIndex = 3;

            argsDesc[7].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
            argsDesc[7].Constant.RootParameterIndex = 4;
            argsDesc[7].Constant.Num32BitValuesToSet = 1;
            argsDesc[7].Constant.DestOffsetIn32BitValues = 0;

            argsDesc[8].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

            D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
            commandSignatureDesc.pArgumentDescs = argsDesc;
            commandSignatureDesc.NumArgumentDescs = _countof(argsDesc);
            commandSignatureDesc.ByteStride = sizeof(IndirectCommand);

            dx12::Device::GetDXDevice()->CreateCommandSignature(&commandSignatureDesc, _spotLightShadowsPipeline.GetRootSignature().Get(), IID_PPV_ARGS(&_cmdSignature));
        }
    }

    void ShadowDrawPass::Setup(rg::RenderPassBuilder& builder)
    {
        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        size_t lightsNum = lightEntities.size();

        for (size_t frameIndex = 0; frameIndex < dx12::BACK_BUFFER_COUNT; ++frameIndex)
        {
            _data.LightCommandBuffers[frameIndex].resize(lightsNum);
            for (size_t i = 0; i < lightsNum; ++i)
            {
                _data.LightCommandBuffers[frameIndex][i] = builder.ReadResource(std::format("ShadowCullBuffer {} (frame {})", i, frameIndex));
            }
        }

        _data.ShadowMaps.resize(lightsNum, rg::ResourceId(-1));
        for (size_t lightIndex = 0; lightIndex < lightsNum; ++lightIndex)
        {
            std::shared_ptr<scene::Entity> entity = lightEntities[lightIndex];
            std::shared_ptr<scene::Light> light = entity->GetComponentAs<scene::Light>("Light");

            if (light->CastShadows)
            {
                _data.ShadowMaps[lightIndex] = builder.WriteResource(std::format("{}_ShadowMap", entity->GetName()));
            }
        }
    }

    void ShadowDrawPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        DrawSpotLightShadows(context, task);
        DrawPointLightShadows(context, task);
    }

    void ShadowDrawPass::DrawSpotLightShadows(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Shadow pass command list - draw");

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Shadow Pass (spot lights) | Draw");
        {
            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_spotLightShadowsPipeline);

            CacheGPU::DataHandle sceneDataHandle = context.GetCache().GetResourcePlacement("SceneCB");
            commandList.SetCBV(0, sceneDataHandle.DataGPU);

            CacheGPU::DataHandle lightsData = context.GetCache().GetResourcePlacement("LightsCB");
            commandList.SetSRV(2, lightsData.DataGPU);

            for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
            {
                std::shared_ptr<scene::Light> light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Spot)
                {
                    continue;
                }

                if (_data.ShadowMaps[lightIndex] == rg::ResourceId(-1))
                {
                    continue;
                }

                PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

                std::shared_ptr<dx12::Resource> shadowMap = context.GetResource(_data.ShadowMaps[lightIndex]);
                std::shared_ptr<dx12::Resource> commandBuffer = context.GetResource(_data.LightCommandBuffers[context.GetFrameIndex()][lightIndex]);

                // Transition resources
                std::vector<dx12::ResourceBarrier> barriers =
                {
                    { commandBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT }
                };
                commandList.TransitionBarriers(barriers);

                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = context.GetCPUHandle(shadowMap->GetAsDSV());
                commandList.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                commandList.SetRenderTargets({ }, &depthHandle);

                commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                std::uint32_t counterBufferOffset = commandBuffer->GetResourceDescription().GetSize().x - sizeof(UINT);
                commandList.ExecuteIndirect(_cmdSignature, objectsNum, *commandBuffer, commandBuffer, 0, counterBufferOffset);

                barriers =
                {
                    { shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE }
                };
                commandList.TransitionBarriers(barriers);

                PIXEndEvent(commandList.GetDXCommandList().Get());
            }
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());
    }

    void ShadowDrawPass::DrawPointLightShadows(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Shadow Pass (point lights) | Draw");
        {
            commandList.SetPipelineState(_pointLightShadowsPipeline);

            CacheGPU::DataHandle sceneDataHandle = context.GetCache().GetResourcePlacement("SceneCB");
            commandList.SetCBV(0, sceneDataHandle.DataGPU);

            CacheGPU::DataHandle lightsData = context.GetCache().GetResourcePlacement("LightsCB");
            commandList.SetSRV(2, lightsData.DataGPU);

            for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
            {
                std::shared_ptr<scene::Light> light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Point)
                {
                    continue;
                }

                if (_data.ShadowMaps[lightIndex] == rg::ResourceId(-1))
                {
                    continue;
                }

                PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

                std::shared_ptr<dx12::Resource> shadowMap = context.GetResource(_data.ShadowMaps[lightIndex]);
                std::shared_ptr<dx12::Resource> commandBuffer = context.GetResource(_data.LightCommandBuffers[context.GetFrameIndex()][lightIndex]);

                // Transition resources
                std::vector<dx12::ResourceBarrier> barriers =
                {
                    { commandBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT }
                };
                commandList.TransitionBarriers(barriers);

                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = context.GetCPUHandle(shadowMap->GetAsDSV());
                commandList.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                commandList.SetRenderTargets({ }, &depthHandle);

                commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                std::uint32_t counterBufferOffset = commandBuffer->GetResourceDescription().GetSize().x - sizeof(UINT);
                commandList.ExecuteIndirect(_cmdSignature, objectsNum, *commandBuffer, commandBuffer, 0, counterBufferOffset);

                barriers =
                {
                    { shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON }
                };
                commandList.TransitionBarriers(barriers);

                PIXEndEvent(commandList.GetDXCommandList().Get());
            }
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
