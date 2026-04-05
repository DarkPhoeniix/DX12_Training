#include "RendererPCH.h"

#include "ShadowDrawPass.h"

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
    ShadowDrawPass::ShadowDrawPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<ShadowDrawPassData>("shadow_draw_pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
        _spotLightShadowsPipeline = _device->CreatePipelineState("PipelineDescriptions\\Shadow_SpotLight.tech");
        _pointLightShadowsPipeline = _device->CreatePipelineState("PipelineDescriptions\\Shadow_PointLight.tech");

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

    void ShadowDrawPass::Execute(rg::RenderContext& context, rg::ITask* task)
    {
        DrawSpotLightShadows(context, task);
        DrawPointLightShadows(context, task);
    }

    void ShadowDrawPass::DrawSpotLightShadows(rg::RenderContext& context, rg::ITask* task)
    {
        rhi::CommandList* commandList = task->GetCommandList();

        std::shared_ptr<rhi::Buffer> frameBuffer = context.GetFrame()->GetBuffer();

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        {
            GPU_SCOPED_EVENT(commandList, "Shadow Draw Pass (spot lights)", 1);

            context.BindBindlessTable(commandList);
            commandList->SetPipelineState(_spotLightShadowsPipeline.get());

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

                std::shared_ptr<rhi::Texture> shadowMap = context.GetTextureManager().GetTexture(light->ShadowMapHandle);
                std::shared_ptr<rhi::Buffer> commandBuffer = context.GetBuffer(_data.LightCommandBuffers[lightIndex]);

                // Transition resources
                commandList->TransitionBarrier({ shadowMap, rhi::ResourceState::Common, rhi::ResourceState::DepthWrite });

                DescriptorHandle depthHandle = context.GetStaticResourceHandle(shadowMap->GetAsDSV());
                commandList->ClearDSV(depthHandle.CpuHandle);

                CD3DX12_VIEWPORT viewport(0.0f, 0.0f,
                    static_cast<float>(shadowMap->GetWidth()),
                    static_cast<float>(shadowMap->GetHeight()));
                CD3DX12_RECT scissorRect(0, 0, LONG_MAX, LONG_MAX);
                commandList->SetViewport(viewport, scissorRect);
                commandList->SetRenderTargets({ }, &depthHandle.CpuHandle);

                commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);

                std::uint32_t counterBufferOffset = commandBuffer->GetSize() - sizeof(UINT);
                commandList->ExecuteIndirect(_cmdSignature, objectsNum, *commandBuffer, commandBuffer, 0, counterBufferOffset);

                commandList->TransitionBarrier({ shadowMap, rhi::ResourceState::DepthWrite, rhi::ResourceState::Common });
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

            commandList->SetPipelineState(_pointLightShadowsPipeline.get());

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

                std::shared_ptr<rhi::Texture> shadowMap = context.GetTextureManager().GetTexture(light->ShadowMapHandle);
                std::shared_ptr<rhi::Buffer> commandBuffer = context.GetResource(_data.LightCommandBuffers[lightIndex]);

                // Transition resources
                commandList->TransitionBarrier({ shadowMap, rhi::ResourceState::Common, rhi::ResourceState::DepthWrite });

                DescriptorHandle depthHandle = context.GetStaticResourceHandle(shadowMap->GetAsDSV());
                commandList->ClearDSV(depthHandle.CpuHandle, D3D12_CLEAR_FLAG_DEPTH);

                CD3DX12_VIEWPORT viewport(0.0f, 0.0f,
                    static_cast<float>(shadowMap->GetResourceDescription().GetSize().x),
                    static_cast<float>(shadowMap->GetResourceDescription().GetSize().y));
                CD3DX12_RECT scissorRect(0, 0, LONG_MAX, LONG_MAX);
                commandList->SetViewport(viewport, scissorRect);
                commandList->SetRenderTargets({ }, &depthHandle.CpuHandle);

                commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);

                std::uint32_t counterBufferOffset = commandBuffer->GetSize() - sizeof(UINT);
                commandList->ExecuteIndirect(_cmdSignature, objectsNum, *commandBuffer, commandBuffer, 0, counterBufferOffset);

                commandList->TransitionBarrier({ shadowMap, rhi::ResourceState::DepthWrite, rhi::ResourceState::Common });
            }
        }

        commandList->Close();
    }
} // namespace render
