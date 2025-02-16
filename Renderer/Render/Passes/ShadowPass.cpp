#include "RendererPCH.h"

#include "ShadowPass.h"

#include "ResourceTable.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/RenderHelpers.h"

#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Mesh.h"

namespace
{
    // Data structure to match the command signature used for ExecuteIndirect.
    struct IndirectCommand
    {
        D3D12_GPU_VIRTUAL_ADDRESS sceneBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS modelBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS bonesBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS verticesBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS lightsBufferAddress;
        UINT LightIndex;

        D3D12_DRAW_ARGUMENTS drawArguments;
    };

    void DrawEntity(std::shared_ptr<scene::Entity> entity, dx12::CommandList& commandList, CacheGPU* frameCache)
    {
        if (scene::Mesh* mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
        {
            CacheGPU::DataHandle modelDescHandle = frameCache->GetResourcePlacement(entity->GetName());
            GPUModelDesc* desc = (GPUModelDesc*)modelDescHandle.DataCPU;
            commandList.SetCBV(1, modelDescHandle.DataGPU);

            // Update and setup animantion
            if (scene::Armature* armature = entity->GetComponentAs<scene::Armature>("Armature"))
            {
                CacheGPU::DataHandle bonesDescHandle = frameCache->GetResourcePlacement(entity->GetName() + "_bones");
                commandList.SetSRV(3, bonesDescHandle.DataGPU);
            }

            commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList.SetVertexBuffer(0, mesh->VertexBufferView);
            if (!mesh->SkinningVertexData.empty())
            {
                commandList.SetVertexBuffer(1, mesh->SkinningVertexBufferView);
            }
            commandList.SetIndexBuffer(mesh->IndexBufferView);

            commandList.DrawIndexed(mesh->IndexData.size());
        }

        for (std::shared_ptr<scene::Entity>& child : entity->GetChildrenNodes())
        {
            DrawEntity(child, commandList, frameCache);
        }
    }
} // namespace unnamed

namespace render
{
    void ShadowPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _shadowSpotLightPipeline.Parse("PipelineDescriptions\\Shadow_SpotLight.tech");
        _shadowPointLightPipeline.Parse("PipelineDescriptions\\Shadow_PointLight.tech");
        _pointLightCullingPipeline.Parse("PipelineDescriptions\\LightCulling_PointLight.tech");

        {
            D3D12_INDIRECT_ARGUMENT_DESC argsDesc[7];
            argsDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
            argsDesc[0].ConstantBufferView.RootParameterIndex = 0;

            argsDesc[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
            argsDesc[1].ConstantBufferView.RootParameterIndex = 1;

            argsDesc[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
            argsDesc[2].ShaderResourceView.RootParameterIndex = 2;

            argsDesc[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
            argsDesc[3].ShaderResourceView.RootParameterIndex = 3;

            argsDesc[4].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
            argsDesc[4].ShaderResourceView.RootParameterIndex = 4;

            argsDesc[5].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
            argsDesc[5].Constant.RootParameterIndex = 5;
            argsDesc[5].Constant.Num32BitValuesToSet = 1;
            argsDesc[5].Constant.DestOffsetIn32BitValues = 0;

            argsDesc[6].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;


            D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
            commandSignatureDesc.pArgumentDescs = argsDesc;
            commandSignatureDesc.NumArgumentDescs = _countof(argsDesc);
            commandSignatureDesc.ByteStride = sizeof(IndirectCommand);

            dx12::Device::GetDXDevice()->CreateCommandSignature(&commandSignatureDesc, _shadowPointLightPipeline.GetRootSignature().Get(), IID_PPV_ARGS(&_cmdSignature));

        }


        {
            dx12::DescriptorHeapDescription desc;
            desc.SetNumDescriptors(3);
            desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

            _commandsDescHeap.Create(desc);
            _commandsDescHeap.SetName("Test desc heap - execute indirect");
        }

        {
            D3D12_RESOURCE_DESC counterDesc = {};
            counterDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            counterDesc.Width = sizeof(UINT);  // Only need 4 bytes for the counter
            counterDesc.Height = 1;
            counterDesc.DepthOrArraySize = 1;
            counterDesc.MipLevels = 1;
            counterDesc.Format = DXGI_FORMAT_UNKNOWN;
            counterDesc.SampleDesc.Count = 1;
            counterDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            counterDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

            {
                dx12::ResourceDescription desc;
                desc.SetSize({ sizeof(UINT), 1 });
                desc.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
                desc.SetResourceType(dx12::EResourceType::Buffer | dx12::EResourceType::Unordered);

                _counters.push_back(dx12::Resource());
                _counters.back().CreateCommitedResource(desc);
                _counters.back().SetName("Counter buffer 0");

                _counters.push_back(dx12::Resource());
                _counters.back().CreateCommitedResource(desc);
                _counters.back().SetName("Counter buffer 1");

                _counters.push_back(dx12::Resource());
                _counters.back().CreateCommitedResource(desc);
                _counters.back().SetName("Counter buffer 2");

                desc.SetFlags(D3D12_RESOURCE_FLAG_NONE);
                desc.SetResourceType(dx12::EResourceType::Buffer | dx12::EResourceType::ReadBack);

                _counterReadBack.CreateCommitedResource(desc);
                _counterReadBack.SetName("Counter buffer readback");
            }

            {

                dx12::ResourceDescription cDesc;
                cDesc.SetSize({ sizeof(UINT), 1 });
                cDesc.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
                cDesc.SetResourceType(dx12::EResourceType::Buffer | dx12::EResourceType::Dynamic);

                _counterReset.CreateCommitedResource(cDesc, D3D12_RESOURCE_STATE_COPY_SOURCE);
                _counterReset.SetName("Counter reset buffer");
                UINT* val = (UINT*)_counterReset.Map();
                val[0] = 0;
            }

            dx12::ResourceDescription desc;
            desc.SetSize({ 64 * sizeof(IndirectCommand) + sizeof(UINT), 1});
            desc.SetFormat(DXGI_FORMAT_UNKNOWN);
            desc.SetStride(sizeof(IndirectCommand));
            desc.SetResourceType(dx12::EResourceType::Buffer | dx12::EResourceType::Unordered);

            _commandsBuffers.push_back(dx12::Resource());
            _commandsBuffers.back().CreateCommitedResource(desc);
            _commandsBuffers.back().SetName("Test res - execute indirect 0");
            _commandsBuffers.back().SetUAVCounterOffset(4096);

            dx12::Device::CreateUnorderedAccessView(_commandsBuffers.back().GetAsUAV(), _commandsDescHeap, &_commandsBuffers.back());

            _commandsBuffers.push_back(dx12::Resource());
            _commandsBuffers.back().CreateCommitedResource(desc);
            _commandsBuffers.back().SetName("Test res - execute indirect 1");
            _commandsBuffers.back().SetUAVCounterOffset(4096);

            dx12::Device::CreateUnorderedAccessView(_commandsBuffers.back().GetAsUAV(), _commandsDescHeap, &_commandsBuffers.back());

            _commandsBuffers.push_back(dx12::Resource());
            _commandsBuffers.back().CreateCommitedResource(desc);
            _commandsBuffers.back().SetName("Test res - execute indirect 2");
            _commandsBuffers.back().SetUAVCounterOffset(4096);

            dx12::Device::CreateUnorderedAccessView(_commandsBuffers.back().GetAsUAV(), _commandsDescHeap, &_commandsBuffers.back());
        }
    }

    void ShadowPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void ShadowPass::Execute()
    {
        SpotLightsPass();
        PointLightsPass();
    }

    void ShadowPass::SpotLightsPass()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_shadowSpotLightPipeline);
        task->SetName("shadows_spot");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Shadow pass (spot lights) command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 12, "Shadow Pass - Spot lights");
        {
            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::ResourceTable& frameTable = _frame->GetResourceTable();

            auto lightEntities = _scene->FilterNodesByComponent("Light");
            for (uint32_t i = 0; i < lightEntities.size(); ++i)
            {
                scene::Light* light = lightEntities[i]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Spot)
                {
                    continue;
                }

                commandList.SetPipelineState(_shadowSpotLightPipeline);

                std::shared_ptr<dx12::Resource> shadowMap = light->ShadowMap;
                if (!shadowMap)
                {
                    break;
                }

                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::DSV, sceneTable);
                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::SRV, sceneTable);
                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(shadowMap.get(), dx12::ResourceViewType::DSV);

                commandList.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);

                commandList.ClearDSV(depthHandle, D3D12_CLEAR_FLAG_DEPTH);
                commandList.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                commandList.SetRenderTargets({ }, &depthHandle);

                helpers::SetupSceneDataGPU(*_scene, commandList, _frame);

                commandList.SetConstant(4, i);

                for (std::shared_ptr<scene::Entity>& node : _scene->GetRootNodes())
                {
                    DrawEntity(node, commandList, &_frame->GetCache());
                }

                commandList.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    void ShadowPass::PointLightsPass()
    {
        TaskGPU* computeTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_shadowPointLightPipeline);
        computeTask->SetName("shadows_point_compute");
        computeTask->AddDependency("shadows_spot");
        _tasks.push_back(computeTask);

        dx12::CommandList& computeCmd = *computeTask->GetCommandLists().front();
        computeCmd.SetName("Shadow pass (point lights) command list - test");


        TaskGPU* drawTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_shadowPointLightPipeline);
        drawTask->SetName("shadows_point_draw");
        drawTask->AddDependency("shadows_point_compute");
        _tasks.push_back(drawTask);

        dx12::CommandList& drawCmd = *drawTask->GetCommandLists().front();
        drawCmd.SetName("Shadow pass (point lights) command list - draw");


        dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
        dx12::ResourceTable& frameTable = _frame->GetResourceTable();

        dx12::Resource& commandBuffer = _commandsBuffers[_frame->Index];
        dx12::Resource& counter = _counters[_frame->Index];

        {
            frameTable.CopyDescriptor(&commandBuffer, dx12::ResourceViewType::UAV, _commandsDescHeap.GetCPUHandleWithOffset(_frame->Index));
        }

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        for (uint32_t i = 0; i < lightEntities.size(); ++i)
        {
            scene::Light* light = lightEntities[i]->GetComponentAs<scene::Light>("Light");

            std::vector<IndirectCommand> commands;
            CacheGPU::DataHandle counterHandle = _frame->GetCache().RequestPlacement(std::format("LightCoutner {}", i), 4);

            PIXBeginEvent(computeCmd.GetDXCommandList().Get(), 1, "Shadow Pass - Point lights - test");
            {
                // Record all draw command to the commandBuffer
                size_t objectsNum = _scene->GetRootNodes().size();

                CacheGPU::DataHandle sceneAddress = _frame->GetCache().GetResourcePlacement("SceneCB");
                CacheGPU::DataHandle lightsAddress = _frame->GetCache().GetResourcePlacement("LightsCB");

                CacheGPU::DataHandle inputCommands = _frame->GetCache().RequestPlacement("InputCmd", objectsNum * sizeof(IndirectCommand));
                for (size_t j = 0; j < objectsNum; ++j)
                {
                    scene::Mesh* mesh = _scene->GetRootNodes()[j]->GetComponentAs<scene::Mesh>("Mesh");

                    if (!mesh)
                    {
                        continue;
                    }

                    CacheGPU::DataHandle modelAddress = _frame->GetCache().GetResourcePlacement(_scene->GetRootNodes()[j]->GetName());
                    CacheGPU::DataHandle bonesAddress = _frame->GetCache().GetResourcePlacement(_scene->GetRootNodes()[j]->GetName() + "_bones");

                    IndirectCommand com;

                    com.sceneBufferAddress = sceneAddress.DataGPU;
                    com.lightsBufferAddress = lightsAddress.DataGPU;
                    com.verticesBufferAddress = mesh->VertexBuffer->OffsetGPU(0);
                    com.modelBufferAddress = modelAddress.DataGPU;
                    com.bonesBufferAddress = bonesAddress.DataCPU ? bonesAddress.DataGPU : -1;
                    com.LightIndex = i;

                    com.drawArguments.VertexCountPerInstance = mesh->VertexData.size();
                    com.drawArguments.InstanceCount = 1;
                    com.drawArguments.StartVertexLocation = 0;
                    com.drawArguments.StartInstanceLocation = 0;

                    commands.push_back(com);

                    IndirectCommand* list = (IndirectCommand*)inputCommands.DataCPU;
                    list[commands.size() - 1] = com;
                }

                // Write all models bounding boxes to the buffer
                CacheGPU::DataHandle AABBs = _frame->GetCache().RequestPlacement("AABBs", commands.size() * sizeof(scene::AABBVolume));
                for (size_t j = 0, count = 0; j < objectsNum; ++j)
                {
                    if (scene::Mesh* mesh = _scene->GetRootNodes()[j]->GetComponentAs<scene::Mesh>("Mesh"))
                    {
                        scene::Transformation* t = _scene->GetRootNodes()[j]->GetComponentAs<scene::Transformation>("Transformation");

                        scene::AABBVolume* aabb = (scene::AABBVolume*)AABBs.DataCPU;
                        aabb[count++].Min = DirectX::XMVector3TransformCoord(mesh->AABB.Min, t->Transform);
                        aabb[count++].Max = DirectX::XMVector3TransformCoord(mesh->AABB.Max, t->Transform);
                    }
                }

                // Transition resources
                computeCmd.TransitionBarrier(commandBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

                // Reset commands counter
                computeCmd.CopyBufferRegion(_counterReset, commandBuffer, sizeof(UINT), 0, 64 * sizeof(IndirectCommand));

                // Transition resources
                computeCmd.TransitionBarrier(commandBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

                // Setup pipeline
                computeCmd.SetPipelineState(_pointLightCullingPipeline);
                _frame->BindDescriptorHeaps(computeCmd);

                // Setup root signature
                computeCmd.SetConstant(2, commands.size());
                computeCmd.SetSRV(3, AABBs.DataGPU);
                computeCmd.SetSRV(4, inputCommands.DataGPU);
                computeCmd.SetDescriptorTable(5, frameTable.GetResourceGPUHandle(&commandBuffer, dx12::ResourceViewType::UAV));

                // Dispatch culling compute shader
                int xThreadGroups = (uint32_t)std::ceilf(commands.size() / 4.0f);
                computeCmd.Dispatch(xThreadGroups);

                // Transition resources
                computeCmd.TransitionBarrier(commandBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

            }
            PIXEndEvent(computeCmd.GetDXCommandList().Get());


            PIXBeginEvent(drawCmd.GetDXCommandList().Get(), 1, "Shadow Pass - Point lights - draw");
            {
                std::shared_ptr<dx12::Resource> shadowMap = light->ShadowMap;
                if (!shadowMap)
                {
                    break;
                }

                // Copy needed descriptors
                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::DSV, sceneTable);
                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::SRV, sceneTable);

                // Transition resources
                drawCmd.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);

                // Setup pipeline
                drawCmd.SetPipelineState(_shadowPointLightPipeline);

                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(shadowMap.get(), dx12::ResourceViewType::DSV);
                drawCmd.ClearDSV(depthHandle, D3D12_CLEAR_FLAG_DEPTH);
                drawCmd.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                drawCmd.SetRenderTargets({ }, &depthHandle);

                drawCmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                //drawCmd.ExecuteIndirect(_cmdSignature, commands.size(), commandBuffer, *_frame->GetCache().GetCache(), 0, counterHandle.Offset);
                drawCmd.ExecuteIndirect(_cmdSignature, commands.size(), commandBuffer, commandBuffer, 0, 64 * sizeof(IndirectCommand));

                // Transition resources
                drawCmd.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

                commands.clear();
            }
            PIXEndEvent(drawCmd.GetDXCommandList().Get());

        }
            computeCmd.Close();
            drawCmd.Close();
    }
} // namespace render
