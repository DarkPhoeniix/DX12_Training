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
        D3D12_GPU_VIRTUAL_ADDRESS vertex0BufferAddress;
        UINT VB0_Size;
        UINT VB0_Stride;
        D3D12_GPU_VIRTUAL_ADDRESS vertex1BufferAddress;
        UINT VB1_Size;
        UINT VB1_Stride;
        D3D12_GPU_VIRTUAL_ADDRESS indexBufferAddress;
        UINT IB_Size;
        UINT IB_Format;
        D3D12_GPU_VIRTUAL_ADDRESS sceneBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS modelBufferAddress;
        D3D12_GPU_VIRTUAL_ADDRESS bonesBufferAddress;
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

    std::uint32_t AlignToUAVCounterOffset(std::uint32_t size)
    {
        return Math::AlignUp(size, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
    }
} // namespace unnamed

namespace render
{
    void ShadowPass::Initialize()
    {
        IRenderPass::Initialize();

        _shadowSpotLightPipeline.Parse("PipelineDescriptions\\Shadow_SpotLight.tech");
        _shadowPointLightPipeline.Parse("PipelineDescriptions\\Shadow_PointLight.tech");
        _pointLightCullingPipeline.Parse("PipelineDescriptions\\LightCulling_PointLight.tech");

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

            dx12::Device::GetDXDevice()->CreateCommandSignature(&commandSignatureDesc, _shadowPointLightPipeline.GetRootSignature().Get(), IID_PPV_ARGS(&_cmdSignature));

        }

        CreateCommandBuffers();
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
            for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
            {
                scene::Light* light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
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

                commandList.SetConstant(4, lightIndex);

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
        TaskGPU* clearTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        clearTask->SetName("shadows_point_clear");
        clearTask->AddDependency("shadows_spot");
        _tasks.push_back(clearTask);

        dx12::CommandList& clearCmd = *clearTask->GetCommandLists().front();
        clearCmd.SetName("Shadow pass (point lights) command list - clear");

        TaskGPU* computeTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_shadowPointLightPipeline);
        computeTask->SetName("shadows_point_compute");
        computeTask->AddDependency("shadows_spot");
        _tasks.push_back(computeTask);

        dx12::CommandList& computeCmd = *computeTask->GetCommandLists().front();
        computeCmd.SetName("Shadow pass (point lights) command list - test");


        TaskGPU* drawTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_shadowPointLightPipeline);
        drawTask->SetName("shadows_point_draw");
        drawTask->AddDependency("shadows_point_compute");
        drawTask->AddDependency("shadows_point_clear");
        _tasks.push_back(drawTask);

        dx12::CommandList& drawCmd = *drawTask->GetCommandLists().front();
        drawCmd.SetName("Shadow pass (point lights) command list - draw");

        dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
        dx12::ResourceTable& frameTable = _frame->GetResourceTable();

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");


        PIXBeginEvent(clearCmd.GetDXCommandList().Get(), 1, "Shadow Pass - CLEAN!");
        for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            scene::Light* light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
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
                clearCmd.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);

                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(shadowMap.get(), dx12::ResourceViewType::DSV);
                clearCmd.ClearDSV(depthHandle, D3D12_CLEAR_FLAG_DEPTH);
            }
        }
        PIXEndEvent(clearCmd.GetDXCommandList().Get());
        clearCmd.Close();

        PIXBeginEvent(computeCmd.GetDXCommandList().Get(), 1, "Shadow Pass - Point lights - test");
        PIXBeginEvent(drawCmd.GetDXCommandList().Get(), 1, "Shadow Pass - Point lights - draw");
        for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            dx12::Resource& commandBuffer = _commandsBuffers[_frame->Index][lightIndex];
            dx12::Resource& counter = _counters[_frame->Index][lightIndex];

            {
                frameTable.CopyDescriptor(&commandBuffer, dx12::ResourceViewType::UAV, _commandsDescHeap.GetCPUHandleWithOffset(_frame->Index * lightEntities.size() + lightIndex));
            }

            scene::Light* light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");

            std::vector<IndirectCommand> commands;

            {
                // Record all draw command to the commandBuffer
                std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
                size_t objectsNum = meshes.size();

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

                    com.vertex0BufferAddress = mesh->VertexBufferView.BufferLocation;
                    com.VB0_Size = mesh->VertexBufferView.SizeInBytes;
                    com.VB0_Stride = mesh->VertexBufferView.StrideInBytes;
                    com.vertex1BufferAddress = mesh->VertexBufferView.BufferLocation;
                    com.VB1_Size = mesh->VertexBufferView.SizeInBytes;
                    com.VB1_Stride = mesh->VertexBufferView.StrideInBytes;
                    com.indexBufferAddress = mesh->IndexBufferView.BufferLocation;
                    com.IB_Size = mesh->IndexBufferView.SizeInBytes;
                    com.IB_Format = mesh->IndexBufferView.Format;
                    com.sceneBufferAddress = sceneAddress.DataGPU;
                    com.lightsBufferAddress = lightsAddress.DataGPU;
                    com.modelBufferAddress = modelAddress.DataGPU;
                    com.bonesBufferAddress = bonesAddress.DataCPU ? bonesAddress.DataGPU : modelAddress.DataGPU;
                    com.LightIndex = lightIndex;

                    com.drawArguments.VertexCountPerInstance = mesh->VertexData.size();
                    com.drawArguments.InstanceCount = 1;
                    com.drawArguments.StartVertexLocation = 0;
                    com.drawArguments.StartInstanceLocation = 0;

                    commands.push_back(com);

                    IndirectCommand* list = (IndirectCommand*)inputCommands.DataCPU;
                    list[commands.size() - 1] = com;
                }

                // Write all models bounding boxes to the buffer
                CacheGPU::DataHandle AABBs = _frame->GetCache().RequestPlacement("AABBs", commands.size() * sizeof(DirectX::XMVECTOR) * 2);
                for (size_t j = 0, count = 0; j < objectsNum; ++j)
                {
                    if (scene::Mesh* mesh = _scene->GetRootNodes()[j]->GetComponentAs<scene::Mesh>("Mesh"))
                    {
                        scene::Transformation* t = _scene->GetRootNodes()[j]->GetComponentAs<scene::Transformation>("Transformation");
                        
                        DirectX::XMVECTOR* data = (DirectX::XMVECTOR*)AABBs.DataCPU;
                        scene::AABBVolume aabb = mesh->AABB.Transform(t->Transform);

                        data[count++] = aabb.Min;
                        data[count++] = aabb.Max;
                    }
                }

                // Transition resources
                computeCmd.TransitionBarrier(commandBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

                // Reset commands counter
                uint32_t counterBufferOffset = 4096;
                computeCmd.CopyBufferRegion(_counterReset, commandBuffer, sizeof(UINT), 0, counterBufferOffset);

                // Transition resources
                computeCmd.TransitionBarrier(commandBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

                // Setup pipeline
                computeCmd.SetPipelineState(_pointLightCullingPipeline);
                _frame->BindDescriptorHeaps(computeCmd);

                helpers::SetupSceneDataGPU(*_scene, computeCmd, _frame);

                // Setup root signature
                computeCmd.SetConstant(3, commands.size());
                computeCmd.SetSRV(4, AABBs.DataGPU);
                computeCmd.SetSRV(5, inputCommands.DataGPU);
                computeCmd.SetDescriptorTable(6, frameTable.GetResourceGPUHandle(&commandBuffer, dx12::ResourceViewType::UAV));

                // Dispatch culling compute shader
                int xThreadGroups = (uint32_t)std::ceilf(commands.size() / 4.0f);
                computeCmd.Dispatch(xThreadGroups);

                // Transition resources
                computeCmd.TransitionBarrier(commandBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

            }


            {
                std::shared_ptr<dx12::Resource> shadowMap = light->ShadowMap;
                if (!shadowMap)
                {
                    break;
                }

                // Setup pipeline
                drawCmd.SetPipelineState(_shadowPointLightPipeline);

                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(shadowMap.get(), dx12::ResourceViewType::DSV);
                drawCmd.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                drawCmd.SetRenderTargets({ }, &depthHandle);

                drawCmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                uint32_t commandBufferOffset = (64 * sizeof(IndirectCommand) + 4) * lightIndex;
                uint32_t counterBufferOffset = (64 * sizeof(IndirectCommand) + 4) * (lightIndex + 1) - 4;
                drawCmd.ExecuteIndirect(_cmdSignature, commands.size(), commandBuffer, commandBuffer, 0, 4096);

                // Transition resources
                drawCmd.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

                commands.clear();
            }

        }
        PIXEndEvent(drawCmd.GetDXCommandList().Get());
        PIXEndEvent(computeCmd.GetDXCommandList().Get());
        computeCmd.Close();
        drawCmd.Close();
    }

    void ShadowPass::CreateCommandBuffers()
    {
        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshEntities = _scene->FilterNodesByComponent("Mesh");

        std::uint32_t lightsNum = lightEntities.size();
        std::uint32_t meshesNum = meshEntities.size();

        {
            // TODO: hardcoded size
            dx12::HeapDescription heapDescription;
            heapDescription.SetSize(_16MB);
            heapDescription.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);

            _commandsHeap.Create(heapDescription);
        }

        {
            dx12::DescriptorHeapDescription desc;
            desc.SetNumDescriptors(lightsNum * 3);
            desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

            _commandsDescHeap.Create(desc);
            _commandsDescHeap.SetName("Command buffers descriptor heap");
        }

        {
            dx12::ResourceDescription counterDescription;
            counterDescription.SetSize({ sizeof(UINT), 1 });
            counterDescription.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
            counterDescription.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);

            for (size_t frameIndex = 0; frameIndex < 3; ++frameIndex)
            {
                _counters[frameIndex].resize(lightsNum);

                for (size_t lightIndex = 0; lightIndex < lightsNum; ++lightIndex)
                {
                    _counters[frameIndex][lightIndex].SetResourceDescription(counterDescription);
                    _counters[frameIndex][lightIndex].SetName(std::format("Command buffer counter {} (frame {})", lightIndex, frameIndex));
                    _commandsHeap.PlaceResource(_counters[frameIndex][lightIndex], D3D12_RESOURCE_STATE_COMMON);

                }
            }
        }

        {
            dx12::ResourceDescription counterResetBuffer;
            counterResetBuffer.SetSize({ sizeof(UINT), 1 });
            counterResetBuffer.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
            counterResetBuffer.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);

            _counterReset.CreateCommitedResource(counterResetBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
            _counterReset.SetName("Counter reset buffer");
            UINT* val = (UINT*)_counterReset.Map();
            val[0] = 0;
        }

        {
            dx12::ResourceDescription commandsBufferDescription;
            commandsBufferDescription.SetSize({ (AlignToUAVCounterOffset(meshesNum * sizeof(IndirectCommand)) + (uint32_t)sizeof(UINT)), 1 });
            commandsBufferDescription.SetStride(D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
            commandsBufferDescription.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);

            for (size_t frameIndex = 0; frameIndex < 3; ++frameIndex)
            {
                _commandsBuffers[frameIndex].resize(lightsNum);

                for (size_t lightIndex = 0; lightIndex < lightsNum; ++lightIndex)
                {
                    _commandsBuffers[frameIndex][lightIndex].SetResourceDescription(commandsBufferDescription);
                    _commandsBuffers[frameIndex][lightIndex].SetUAVCounterOffset(AlignToUAVCounterOffset(meshesNum * sizeof(IndirectCommand)));
                    _commandsBuffers[frameIndex][lightIndex].SetName(std::format("Commands buffer {} (frame {})", lightIndex, frameIndex));
                    _commandsHeap.PlaceResource(_commandsBuffers[frameIndex][lightIndex], D3D12_RESOURCE_STATE_COMMON);

                    dx12::Device::CreateUnorderedAccessView(_commandsBuffers[frameIndex][lightIndex].GetAsUAV(), _commandsDescHeap, &_commandsBuffers[frameIndex][lightIndex]);
                }
            }
        }
    }
} // namespace render