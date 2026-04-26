#include "RendererPCH.h"

#include "ShadowDrawPass.h"

#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

#include "RHI/ResourceBarrier.h"

namespace
{
    constexpr std::uint32_t CULLING_PASS_THREADS_NUM = 16;
    constexpr std::uint32_t MAX_INSTANCES_NUM = 1u << 20;

    // Data structure to match the command signature used for ExecuteIndirect.
    struct alignas(16) IndirectCommand
    {
        std::uint64_t VertexBufferAddress;
        std::uint32_t VertexBufferSize;
        std::uint32_t VertexBufferStride;
        std::uint64_t SkinBufferAddress;
        std::uint32_t SkinBufferSize;
        std::uint32_t SkinBufferStride;
        std::uint64_t IndexBufferAddress;
        std::uint32_t IndexBufferSize;
        std::uint32_t IndexBufferFormat;

        std::uint64_t FrameBufferAddress;
        std::uint32_t InstanceIndex;
        std::uint32_t LightIndex;

        std::uint32_t IndexCountPerInstance;
        std::uint32_t InstanceCount;
        std::uint32_t StartIndexLocation;
        std::int32_t BaseVertexLocation;
        std::uint32_t StartInstanceLocation;
    };

    std::uint32_t AlignToUAVCounterOffset(std::uint32_t size)
    {
        return Math::AlignUp(size, 4096); // D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT
    }
} // namespace unnamed

namespace render
{
    ShadowDrawPass::ShadowDrawPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<ShadowDrawPassData>(device, "shadow_draw_pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
        _spotLightShadowsPipeline = _device->CreatePipelineState("PipelineDescriptions\\Shadow_SpotLight.tech");
        _pointLightShadowsPipeline = _device->CreatePipelineState("PipelineDescriptions\\Shadow_PointLight.tech");

        {
            // https://microsoft.github.io/DirectX-Specs/d3d/IndirectDrawing.html#root-constants--vertex-buffers
            std::vector<rhi::IndirectArgumentDescription> args(6);

            args[0].Type = rhi::IndirectArgumentType::VertexBufferView;
            args[0].VertexBuffer.Slot = 0;

            args[1].Type = rhi::IndirectArgumentType::VertexBufferView;
            args[1].VertexBuffer.Slot = 1;

            args[2].Type = rhi::IndirectArgumentType::IndexBufferView;

            args[3].Type = rhi::IndirectArgumentType::ConstantBufferView;
            args[3].ConstantBufferView.RootParameterIndex = 0;

            args[4].Type = rhi::IndirectArgumentType::Constant;
            args[4].Constant.RootParameterIndex = 1;
            args[4].Constant.Num32BitValuesToSet = 2;
            args[4].Constant.DestOffsetIn32BitValues = 0;

            args[5].Type = rhi::IndirectArgumentType::DrawIndexed;

            _cmdSignature = device->CreateCommandSignature(args, _pointLightShadowsPipeline.get(), "shadow_draw_command_signature");
        }
    }

    void ShadowDrawPass::Setup(rg::RenderPassBuilder& builder)
    {
        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        size_t lightsNum = lightEntities.size();

        _data.LightCommandBuffers.resize(lightsNum);
        _data.ShadowMaps.resize(lightsNum);
        for (size_t i = 0; i < lightsNum; ++i)
        {
            _data.LightCommandBuffers[i] = builder.IndirectArgBuffer(std::format("shadow_culled_instances_buffer_{}", i));
            _data.ShadowMaps[i] = builder.WriteVirtualResource(lightEntities[i]->GetName() + "_shadow_map");
        }
    }

    void ShadowDrawPass::Execute(rg::RenderContext& context, rg::ITask* task)
    {
        DrawSpotLightShadows(context, task);
        DrawPointLightShadows(context, task);
    }

    void ShadowDrawPass::DrawSpotLightShadows(rg::RenderContext& context, rg::ITask* task)
    {
        rhi::CommandList* commandList = task->GetCommandList();

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        {
            GPU_SCOPED_EVENT(commandList, "Shadow Draw Pass (spot lights)", 1);

            commandList->SetGraphicsPipelineState(_spotLightShadowsPipeline.get());

            for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
            {
                GPU_SCOPED_EVENT(commandList, lightEntities[lightIndex]->GetName().c_str(), 1);

                std::shared_ptr<scene::Light> light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Spot)
                {
                    continue;
                }

                if (!light->CastShadows)
                {
                    continue;
                }

                std::shared_ptr<rhi::Texture> shadowMap = context.GetTexture((rg::RGTextureId)light->ShadowMapId);
                std::shared_ptr<rhi::Buffer> commandBuffer = context.GetBuffer(_data.LightCommandBuffers[lightIndex]);

                // Transition resources
                rhi::TextureBarrier shadowMapBarrier = { shadowMap, rhi::ResourceState::Common, rhi::ResourceState::DepthWrite };
                commandList->TransitionBarriers({ shadowMapBarrier });

                rhi::CPUDescriptor depthHandle = context.GetDescriptor((rg::RGTextureId)shadowMap->GetID(), rhi::ResourceViewType::DSV);
                commandList->ClearDSV(depthHandle);

                rhi::Viewport viewport(0.0f, 0.0f,
                    static_cast<float>(shadowMap->GetWidth()),
                    static_cast<float>(shadowMap->GetHeight()));
                rhi::ScissorRect scissorRect(0, 0, LONG_MAX, LONG_MAX);
                commandList->SetViewport(viewport, scissorRect);
                commandList->SetRenderTargets({ }, &depthHandle);

                commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);

                std::uint32_t counterBufferOffset = commandBuffer->GetSize() - sizeof(UINT);
                commandList->ExecuteIndirect(_cmdSignature.get(), objectsNum, commandBuffer, commandBuffer, 0, counterBufferOffset);

                shadowMapBarrier = { shadowMap, rhi::ResourceState::DepthWrite, rhi::ResourceState::Common };
                commandList->TransitionBarriers({ shadowMapBarrier });
            }
        }
    }

    void ShadowDrawPass::DrawPointLightShadows(rg::RenderContext& context, rg::ITask* task)
    {
        rhi::CommandList* commandList = task->GetCommandList();

        {
            GPU_SCOPED_EVENT(commandList, "Shadow Draw Pass (point lights)", 1);

            std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
            std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
            size_t objectsNum = meshes.size();

            commandList->SetGraphicsPipelineState(_pointLightShadowsPipeline.get());

            for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
            {
                GPU_SCOPED_EVENT(commandList, lightEntities[lightIndex]->GetName().c_str(), 1);

                std::shared_ptr<scene::Light> light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Point)
                {
                    continue;
                }

                if (!light->CastShadows)
                {
                    continue;
                }

                std::shared_ptr<rhi::Texture> shadowMap = context.GetTexture((rg::RGTextureId)light->ShadowMapId);
                std::shared_ptr<rhi::Buffer> commandBuffer = context.GetBuffer(_data.LightCommandBuffers[lightIndex]);

                // Transition resources
                rhi::TextureBarrier shadowMapBarrier = { shadowMap, rhi::ResourceState::Common, rhi::ResourceState::DepthWrite };
                commandList->TransitionBarriers({ shadowMapBarrier });

                rhi::CPUDescriptor depthHandle = context.GetDescriptor((rg::RGTextureId)shadowMap->GetID(), rhi::ResourceViewType::DSV);
                commandList->ClearDSV(depthHandle);

                rhi::Viewport viewport(0.0f, 0.0f,
                    static_cast<float>(shadowMap->GetWidth()),
                    static_cast<float>(shadowMap->GetHeight()));
                rhi::ScissorRect scissorRect(0, 0, LONG_MAX, LONG_MAX);
                commandList->SetViewport(viewport, scissorRect);
                commandList->SetRenderTargets({ }, &depthHandle);

                commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);

                std::uint32_t counterBufferOffset = commandBuffer->GetSize() - sizeof(UINT);
                commandList->ExecuteIndirect(_cmdSignature.get(), objectsNum, commandBuffer, commandBuffer, 0, counterBufferOffset);

                shadowMapBarrier = { shadowMap, rhi::ResourceState::DepthWrite, rhi::ResourceState::Common };
                commandList->TransitionBarriers({ shadowMapBarrier });
            }
        }

        commandList->Close();
    }
} // namespace render
