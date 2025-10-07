#include "RendererPCH.h"

#include "ShadowDrawPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
    constexpr std::uint32_t CULLING_PASS_THREADS_NUM = 16;
    constexpr std::uint32_t MAX_INSTANCES_NUM = 1u << 20;

    // Data structure to match the command signature used for ExecuteIndirect.
    struct alignas(16) IndirectCommand
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

        D3D12_GPU_VIRTUAL_ADDRESS FrameBufferAddress;
        UINT InstanceIndex;
        UINT LightIndex;

        D3D12_DRAW_INDEXED_ARGUMENTS DrawArguments;
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
            std::vector<D3D12_INDIRECT_ARGUMENT_DESC> argsDesc(6);

            argsDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
            argsDesc[0].VertexBuffer.Slot = 0;

            argsDesc[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
            argsDesc[1].VertexBuffer.Slot = 1;

            argsDesc[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;

            argsDesc[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
            argsDesc[3].ConstantBufferView.RootParameterIndex = 0;

            argsDesc[4].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
            argsDesc[4].Constant.RootParameterIndex = 1;
            argsDesc[4].Constant.Num32BitValuesToSet = 2;
            argsDesc[4].Constant.DestOffsetIn32BitValues = 0;

            argsDesc[5].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

            for (const auto& arg : argsDesc)
            {
                _cmdSignature.AddArgument(arg);
            }

            _cmdSignature.Create(sizeof(IndirectCommand), &_pointLightShadowsPipeline);
        }
    }

    void ShadowDrawPass::Setup(rg::RenderPassBuilder& builder)
    {
        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        size_t lightsNum = lightEntities.size();

        _data.LightCommandBuffers.resize(lightsNum);
        for (size_t i = 0; i < lightsNum; ++i)
        {
            _data.LightCommandBuffers[i] = builder.IndirectArgBuffer(std::format("shadow_culled_instances_buffer_{}", i));
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

        std::shared_ptr<dx12::Resource> frameBuffer = context.GetFrame()->GetBuffer();

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Shadow Pass (spot lights) | Draw");
        {
            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_spotLightShadowsPipeline);

            for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
            {
                std::shared_ptr<scene::Light> light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Spot)
                {
                    continue;
                }

                if (!light->CastShadows)
                {
                    continue;
                }

                PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

                std::shared_ptr<dx12::Resource> shadowMap = context.GetTextureManager().GetTexture(light->ShadowMapHandle);
                std::shared_ptr<dx12::Resource> commandBuffer = context.GetResource(_data.LightCommandBuffers[lightIndex]);

                // Transition resources
                commandList.TransitionBarrier({ shadowMap, dx12::ResourceState::Common, dx12::ResourceState::DepthWrite });

                DescriptorHandle depthHandle = context.GetStaticResourceHandle(shadowMap->GetAsDSV());
                commandList.ClearDSV(depthHandle.CpuHandle, D3D12_CLEAR_FLAG_DEPTH);

                commandList.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                commandList.SetRenderTargets({ }, &depthHandle.CpuHandle);

                commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                std::uint32_t counterBufferOffset = commandBuffer->GetResourceDescription().GetSize().x - sizeof(UINT);
                commandList.ExecuteIndirect(_cmdSignature, objectsNum, *commandBuffer, commandBuffer, 0, counterBufferOffset);

                commandList.TransitionBarrier({ shadowMap, dx12::ResourceState::DepthWrite, dx12::ResourceState::Common });

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

            for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
            {
                std::shared_ptr<scene::Light> light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Point)
                {
                    continue;
                }

                if (!light->CastShadows)
                {
                    continue;
                }

                PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

                std::shared_ptr<dx12::Resource> shadowMap = context.GetTextureManager().GetTexture(light->ShadowMapHandle);
                std::shared_ptr<dx12::Resource> commandBuffer = context.GetResource(_data.LightCommandBuffers[lightIndex]);

                // Transition resources
                commandList.TransitionBarrier({ shadowMap, dx12::ResourceState::Common, dx12::ResourceState::DepthWrite });

                DescriptorHandle depthHandle = context.GetStaticResourceHandle(shadowMap->GetAsDSV());
                commandList.ClearDSV(depthHandle.CpuHandle, D3D12_CLEAR_FLAG_DEPTH);

                commandList.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                commandList.SetRenderTargets({ }, &depthHandle.CpuHandle);

                commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                std::uint32_t counterBufferOffset = commandBuffer->GetResourceDescription().GetSize().x - sizeof(UINT);
                commandList.ExecuteIndirect(_cmdSignature, objectsNum, *commandBuffer, commandBuffer, 0, counterBufferOffset);

                commandList.TransitionBarrier({ shadowMap, dx12::ResourceState::DepthWrite, dx12::ResourceState::Common });

                PIXEndEvent(commandList.GetDXCommandList().Get());
            }
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
