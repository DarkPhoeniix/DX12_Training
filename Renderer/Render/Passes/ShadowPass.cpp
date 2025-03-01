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

    void DrawEntity(std::shared_ptr<scene::Entity> entity, dx12::CommandList& commandList, CacheGPU* frameCache)
    {
        if (scene::Mesh* mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
        {
            CacheGPU::DataHandle modelDescHandle = frameCache->GetResourcePlacement(entity->GetName());
            GPUModelDesc* desc = (GPUModelDesc*)modelDescHandle.DataCPU;
            commandList.SetCBV(1, modelDescHandle.DataGPU);

            // Update and setup animation
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
    }

    void ShadowPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void ShadowPass::Execute()
    {
        ClearDepthTargets();

        UpdateCommandBuffers();
        RecordCommandBuffers();

        SpotLightsPass();
        PointLightsPass();
    }

    void ShadowPass::ClearDepthTargets()
    {
        TaskGPU* clearTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        clearTask->SetName("shadows_clear");
        _tasks.push_back(clearTask);

        dx12::CommandList& clearCmd = *clearTask->GetCommandLists().front();
        clearCmd.SetName("Shadow pass (point lights) command list - clear");

        dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
        dx12::ResourceTable& frameTable = _frame->GetResourceTable();

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");

        PIXBeginEvent(clearCmd.GetDXCommandList().Get(), 1, "Shadow Pass | Clear");
        for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            scene::Light* light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
            if (std::shared_ptr<dx12::Resource> shadowMap = light->ShadowMap)
            {
                PIXBeginEvent(clearCmd.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

                // Copy needed descriptors
                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::DSV, sceneTable);
                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::SRV, sceneTable);

                // Transition resources
                clearCmd.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);

                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(shadowMap.get(), dx12::ResourceViewType::DSV);
                clearCmd.ClearDSV(depthHandle, D3D12_CLEAR_FLAG_DEPTH);

                PIXEndEvent(clearCmd.GetDXCommandList().Get());
            }
        }
        PIXEndEvent(clearCmd.GetDXCommandList().Get());

        clearCmd.Close();
    }

    void ShadowPass::UpdateCommandBuffers()
    {
        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshEntities = _scene->FilterNodesByComponent("Mesh");

        std::uint32_t lightsNum = lightEntities.size();
        std::uint32_t meshesNum = meshEntities.size();

        if (!_commandsBuffers[_frame->Index].empty())
        {
            std::uint32_t currentBufferSize = _commandsBuffers[_frame->Index][0].GetResourceDescription().GetSize().x;
            std::uint32_t requiredBufferSize = meshEntities.size() * sizeof(IndirectCommand);
            if (currentBufferSize >= requiredBufferSize)
            {
                return;
            }
        }

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

    void ShadowPass::RecordCommandBuffers()
    {
        TaskGPU* computeTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_shadowPointLightPipeline);
        computeTask->SetName("shadows_compute");
        _tasks.push_back(computeTask);

        dx12::CommandList& computeCmd = *computeTask->GetCommandLists().front();
        computeCmd.SetName("Shadow pass command list - culling");

        dx12::ResourceTable& frameTable = _frame->GetResourceTable();

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        PIXBeginEvent(computeCmd.GetDXCommandList().Get(), 1, "Shadow Pass | Culling");

        CacheGPU::DataHandle sceneAddress = _frame->GetCache().GetResourcePlacement("SceneCB");
        CacheGPU::DataHandle lightsAddress = _frame->GetCache().GetResourcePlacement("LightsCB");

        // Setup pipeline
        computeCmd.SetPipelineState(_pointLightCullingPipeline);
        _frame->BindDescriptorHeaps(computeCmd);

        helpers::SetupSceneDataGPU(*_scene, computeCmd, _frame);

        for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            PIXBeginEvent(computeCmd.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());
            scene::Light* light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");

            dx12::Resource& commandBuffer = _commandsBuffers[_frame->Index][lightIndex];
            frameTable.CopyDescriptor(&commandBuffer, dx12::ResourceViewType::UAV, _commandsDescHeap.GetCPUHandleWithOffset(_frame->Index * lightEntities.size() + lightIndex));

            CacheGPU::DataHandle inputCommands = _frame->GetCache().RequestPlacement(std::format("InputCmd_{}", lightEntities[lightIndex]->GetName()), objectsNum * sizeof(IndirectCommand));
            IndirectCommand* commandsList = (IndirectCommand*)inputCommands.DataCPU;

            // Record all draw command to the commandBuffer

            for (size_t j = 0; j < objectsNum; ++j)
            {
                scene::Mesh* mesh = meshes[j]->GetComponentAs<scene::Mesh>("Mesh");

                if (!mesh)
                {
                    continue;
                }

                CacheGPU::DataHandle modelAddress = _frame->GetCache().GetResourcePlacement(meshes[j]->GetName());
                CacheGPU::DataHandle bonesAddress = _frame->GetCache().GetResourcePlacement(meshes[j]->GetName() + "_bones");

                IndirectCommand command;

                command.VertexBufferAddress = mesh->VertexBufferView.BufferLocation;
                command.VertexBufferSize = mesh->VertexBufferView.SizeInBytes;
                command.VertexBufferStride = mesh->VertexBufferView.StrideInBytes;

                if (!mesh->SkinningVertexData.empty())
                {
                    command.SkinBufferAddress = mesh->SkinningVertexBufferView.BufferLocation;
                    command.SkinBufferSize = mesh->SkinningVertexBufferView.SizeInBytes;
                    command.SkinBufferStride = mesh->SkinningVertexBufferView.StrideInBytes;

                    command.BonesBufferAddress = bonesAddress.DataGPU;
                }
                else
                {
                    command.SkinBufferAddress = mesh->VertexBufferView.BufferLocation;
                    command.SkinBufferSize = mesh->VertexBufferView.SizeInBytes;
                    command.SkinBufferStride = mesh->VertexBufferView.StrideInBytes;

                    command.BonesBufferAddress = modelAddress.DataGPU;
                }

                command.IndexBufferAddress = mesh->IndexBufferView.BufferLocation;
                command.IndexBufferSize = mesh->IndexBufferView.SizeInBytes;
                command.IndexBufferFormat = mesh->IndexBufferView.Format;

                command.SceneBufferAddress = sceneAddress.DataGPU;
                command.LightsBufferAddress = lightsAddress.DataGPU;
                command.ModelBufferAddress = modelAddress.DataGPU;
                command.LightIndex = lightIndex;

                command.DrawArguments.VertexCountPerInstance = mesh->VertexData.size();
                command.DrawArguments.InstanceCount = 1;
                command.DrawArguments.StartVertexLocation = 0;
                command.DrawArguments.StartInstanceLocation = 0;

                commandsList[j] = command;
            }

            // Write all models bounding boxes to the buffer
            CacheGPU::DataHandle AABBs = _frame->GetCache().RequestPlacement("AABBs", objectsNum * sizeof(DirectX::XMVECTOR) * 2);
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
            std::uint32_t counterBufferOffset = commandBuffer.GetResourceDescription().GetSize().x - sizeof(UINT);
            computeCmd.CopyBufferRegion(_counterReset, commandBuffer, sizeof(UINT), 0, counterBufferOffset);

            // Transition resources
            computeCmd.TransitionBarrier(commandBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            // Setup root signature
            computeCmd.SetConstant(3, objectsNum);
            computeCmd.SetSRV(4, AABBs.DataGPU);
            computeCmd.SetSRV(5, inputCommands.DataGPU);
            computeCmd.SetDescriptorTable(6, frameTable.GetResourceGPUHandle(&commandBuffer, dx12::ResourceViewType::UAV));

            // Dispatch culling compute shader
            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(objectsNum / (float)CULLING_PASS_THREADS_NUM);
            computeCmd.Dispatch(xThreadGroups);

            // Transition resources
            computeCmd.TransitionBarrier(commandBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
            PIXEndEvent(computeCmd.GetDXCommandList().Get());
        }
        PIXEndEvent(computeCmd.GetDXCommandList().Get());

        computeCmd.Close();
    }

    void ShadowPass::SpotLightsPass()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_shadowSpotLightPipeline);
        task->SetName("shadows_spot");
        task->AddDependency("shadows_clear");
        task->AddDependency("shadows_compute");
        _tasks.push_back(task);

        dx12::CommandList& drawCmd = *task->GetCommandLists().front();
        drawCmd.SetName("Shadow pass (spot lights) cmdList - draw");

        dx12::ResourceTable& frameTable = _frame->GetResourceTable();

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        PIXBeginEvent(drawCmd.GetDXCommandList().Get(), 1, "Shadow Pass (spot lights) | Draw");
        {
            drawCmd.SetPipelineState(_shadowSpotLightPipeline);

            helpers::SetupSceneDataGPU(*_scene, drawCmd, _frame);

            for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
            {
                scene::Light* light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Spot)
                {
                    continue;
                }

                std::shared_ptr<dx12::Resource> shadowMap = light->ShadowMap;
                if (!shadowMap)
                {
                    continue;
                }

                PIXBeginEvent(drawCmd.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

                dx12::Resource& commandBuffer = _commandsBuffers[_frame->Index][lightIndex];

                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(shadowMap.get(), dx12::ResourceViewType::DSV);
                drawCmd.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                drawCmd.SetRenderTargets({ }, &depthHandle);

                drawCmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                std::uint32_t counterBufferOffset = commandBuffer.GetResourceDescription().GetSize().x - sizeof(UINT);
                drawCmd.ExecuteIndirect(_cmdSignature, objectsNum, commandBuffer, commandBuffer, 0, counterBufferOffset);

                drawCmd.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

                PIXEndEvent(drawCmd.GetDXCommandList().Get());
            }
        }
        PIXEndEvent(drawCmd.GetDXCommandList().Get());

        drawCmd.Close();
    }

    void ShadowPass::PointLightsPass()
    {
        TaskGPU* drawTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_shadowPointLightPipeline);
        drawTask->SetName("shadows_point_draw");
        drawTask->AddDependency("shadows_clear");
        drawTask->AddDependency("shadows_compute");
        _tasks.push_back(drawTask);

        dx12::CommandList& drawCmd = *drawTask->GetCommandLists().front();
        drawCmd.SetName("Shadow pass (point lights) cmdList - draw");

        dx12::ResourceTable& frameTable = _frame->GetResourceTable();

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshes = _scene->FilterNodesByComponent("Mesh");
        size_t objectsNum = meshes.size();

        // Setup pipeline
        drawCmd.SetPipelineState(_shadowPointLightPipeline);

        helpers::SetupSceneDataGPU(*_scene, drawCmd, _frame);

        PIXBeginEvent(drawCmd.GetDXCommandList().Get(), 1, "Shadow Pass (point lights) | Draw");
        for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            scene::Light* light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light");
            if (light->Type != scene::LightType::Point)
            {
                continue;
            }

            std::shared_ptr<dx12::Resource> shadowMap = light->ShadowMap;
            if (!shadowMap)
            {
                continue;
            }

            PIXBeginEvent(drawCmd.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

            dx12::Resource& commandBuffer = _commandsBuffers[_frame->Index][lightIndex];

            D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(shadowMap.get(), dx12::ResourceViewType::DSV);
            drawCmd.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
            drawCmd.SetRenderTargets({ }, &depthHandle);

            drawCmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            std::uint32_t counterBufferOffset = commandBuffer.GetResourceDescription().GetSize().x - sizeof(UINT);
            drawCmd.ExecuteIndirect(_cmdSignature, objectsNum, commandBuffer, commandBuffer, 0, counterBufferOffset);

            // Transition resources
            drawCmd.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

            PIXEndEvent(drawCmd.GetDXCommandList().Get());
        }
        PIXEndEvent(drawCmd.GetDXCommandList().Get());

        drawCmd.Close();
    }
} // namespace render