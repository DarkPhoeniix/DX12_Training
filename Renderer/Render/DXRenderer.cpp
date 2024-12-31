#include "stdafx.h"

#include "DXRenderer.h"

#include "CommandList.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/RenderEvent.h"
#include "Events/UpdateEvent.h"
#include "Events/KeyEvent.h"
#include "GUI/GUI.h"
#include "Render/Frame/TaskGPU.h"
#include "Utility/DebugInfo.h"

#include "Scene/ECS/Components/Armature.h"
#include "Scene/ECS/Components/Transformation.h"
#include "Scene/ECS/Components/Skybox.h"

#include "Scene/Volumes/AABBVolume.h"

using namespace DirectX;
using namespace Core;

namespace
{
    constexpr float MOVE_SPEED = 200.0f;

    const static DirectX::XMVECTOR _kBoxVerts[8] =
    {
        // front rect
        DirectX::XMVectorSet(-1.0f, -1.0f, -1.0f, 1.0f),
        DirectX::XMVectorSet(-1.0f,  1.0f, -1.0f, 1.0f),
        DirectX::XMVectorSet( 1.0f,  1.0f, -1.0f, 1.0f),
        DirectX::XMVectorSet( 1.0f, -1.0f, -1.0f, 1.0f),

        // back rect
        DirectX::XMVectorSet(-1.0f, -1.0f,  1.0f, 1.0f),
        DirectX::XMVectorSet(-1.0f,  1.0f,  1.0f, 1.0f),
        DirectX::XMVectorSet( 1.0f,  1.0f,  1.0f, 1.0f),
        DirectX::XMVectorSet( 1.0f, -1.0f,  1.0f, 1.0f)
    };

    SceneLayer::AABBVolume CombineOBBs(const std::vector<SceneLayer::OBBVolume>& volumes)
    {
        SceneLayer::AABBVolume result;

        for (const auto& volume : volumes)
        {
            if (!DirectX::XMMatrixIsNaN(volume.Bounds))
            {
                for (int i = 0; i < 8; ++i)
                {
                    DirectX::XMVECTOR v = DirectX::XMVector4Transform(_kBoxVerts[i], volume.Bounds);
                    result.Min = DirectX::XMVectorMin(result.Min, v);
                    result.Max = DirectX::XMVectorMax(result.Max, v);
                }
            }
        }

        return result;
    }
} // namespace unnamed

DXRenderer::DXRenderer(HWND windowHandle)
    : _windowHandle(windowHandle)
    , _contentLoaded(false)
    , _currentFrame(nullptr)
    , _isCameraMoving(false)
    , _deltaTime(0.0f)
    , _renderArmature(false)
    , _renderAABB(false)
    , _applyFXAA(false)
    , _renderSkybox(true)
    , _timeMiltiplier(1.0f)
{   }

DXRenderer::~DXRenderer()
{   }

bool DXRenderer::LoadContent(TaskGPU* loadTask)
{
    _gPassPipeline.Parse("PipelineDescriptions\\GPassPipeline.tech");
    _deferredPipeline.Parse("PipelineDescriptions\\DeferredShading.tech");
    _AABBpipeline.Parse("PipelineDescriptions\\AABBRenderPipeline.tech");
    _OBBpipeline.Parse("PipelineDescriptions\\OBBRenderPipeline.tech");
    _SkyboxPipeline.Parse("PipelineDescriptions\\SkyboxPipeline.tech");
    _FXAAPipeline.Parse("PipelineDescriptions\\FXAAPipeline.tech");
    _ArmatureDebugPipeline.Parse("PipelineDescriptions\\ArmatureDebugPipeline.tech");

    RECT windowSize;
    GetWindowRect(_windowHandle, &windowSize);
    uint32_t windowWidth = windowSize.right - windowSize.left;
    uint32_t windowHeight = windowSize.bottom - windowSize.top;

    // Camera Setup
    {
        XMVECTOR pos = XMVectorSet(15.0f, 23.0f, 20.0f, 1.0f);
        XMVECTOR target = XMVectorSet(0.0f, 20.0f, 0.0f, 1.0f);
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        
        _camera.LookAt(pos, target, up);
        _camera.SetViewport(SceneLayer::Viewport({ windowWidth, windowHeight }));
        _camera.SetLens(45.0f, 0.1f, 1000.0f);
    }

    _gBuffer.Init({ windowWidth, windowHeight });

    // Load scene
    {
        loadTask->SetName("Upload Data");
        dx12::CommandList& commandList = *loadTask->GetCommandLists().front();

        _scene.LoadScene("Dragon\\DragonScene.scene", commandList);
        _uploadProcessor.Process(_scene, commandList);
        _scene.SetCamera(_camera);

        commandList.Close();
    }

    _contentLoaded = true;
    return _contentLoaded;
}

void DXRenderer::UnloadContent()
{
    _contentLoaded = false;
}

void DXRenderer::OnUpdate(Events::UpdateEvent& updateEvent)
{
    DebugInfo::Update(updateEvent);

    _scene.GetCache().SetTime(updateEvent.totalTime * _timeMiltiplier);

    XMVECTOR mov = 50.0f * XMVectorSet(sinf(updateEvent.totalTime * _timeMiltiplier * 0.45f), 0.8f, cosf(updateEvent.totalTime * _timeMiltiplier * 0.45f), 1.0f);
    XMVECTOR tar = XMVectorSet(0.0f, 17.0f, 0.0f, 1.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    //_camera.LookAt(mov, tar, up);

    _deltaTime = updateEvent.elapsedTime * _timeMiltiplier;
}

void DXRenderer::OnRender(Events::RenderEvent& renderEvent, Frame& frame)
{
    _currentFrame = &frame;

    _currentFrame->WaitCPU();
    _currentFrame->ResetGPU();
    _currentFrame->ResetCache();

    // Clear render targets
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("clean");

        ClearBuffers(*task);
    }

    // Execute the GBuffer Pass
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_gPassPipeline);
        task->SetName("g-pass");
        task->AddDependency("clean");

        GeometryPass(*task);
    }

    // Execute the Deferred Shading
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_deferredPipeline);
        task->SetName("deferred");
        task->AddDependency("clean");
        task->AddDependency("g-pass");

        LightingPass(*task);
    }

    // Execute the Skybox
    if (_renderSkybox)
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_SkyboxPipeline);
        task->SetName("skybox");
        task->AddDependency("clean");
        task->AddDependency("g-pass");
        task->AddDependency("deferred");

        RenderSkybox(*task);
    }

    //{
    //    TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
    //    task->SetName("transit1");
    //    task->AddDependency("clean");
    //    task->AddDependency("g-pass");
    //    task->AddDependency("deferred");
    //    task->AddDependency("skybox");

    //    dx12::CommandList& commandList = *task->GetCommandLists().front();

    //    commandList.TransitionBarrier(_currentFrame->_targetTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    //    commandList.TransitionBarrier(_currentFrame->_fxaaTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    //    commandList.Close();
    //}



    //// Execute the FXAA
    //if (_applyFXAA)
    //{
    //    TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_FXAAPipeline);
    //    task->SetName("fxaa");
    //    task->AddDependency("clean");
    //    task->AddDependency("g-pass");
    //    task->AddDependency("deferred");
    //    task->AddDependency("skybox");
    //    task->AddDependency("transit1");

    //    RenderFXAA(*task);
    //}

    //{
    //    TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
    //    task->SetName("transit2");
    //    task->AddDependency("clean");
    //    task->AddDependency("g-pass");
    //    task->AddDependency("deferred");
    //    task->AddDependency("skybox");
    //    task->AddDependency("transit1");
    //    task->AddDependency("fxaa");

    //    dx12::CommandList& commandList = *task->GetCommandLists().front();

    //    commandList.TransitionBarrier(_currentFrame->_targetTexture, D3D12_RESOURCE_STATE_COPY_DEST);
    //    commandList.TransitionBarrier(_currentFrame->_fxaaTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);

    //    if (_applyFXAA)
    //        commandList.CopyResource(_currentFrame->_fxaaTexture, _currentFrame->_targetTexture);

    //    commandList.TransitionBarrier(_currentFrame->_targetTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //    commandList.TransitionBarrier(_currentFrame->_fxaaTexture, D3D12_RESOURCE_STATE_COMMON);

    //    commandList.Close();
    //}

    //// Render Armature
    //if (_renderArmature)
    //{
    //    TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_ArmatureDebugPipeline);
    //    task->SetName("armature");
    //    task->AddDependency("clean");
    //    task->AddDependency("g-pass");
    //    task->AddDependency("deferred");
    //    task->AddDependency("skybox");
    //    task->AddDependency("transit1");
    //    task->AddDependency("fxaa");
    //    task->AddDependency("transit2");

    //    RenderArmature(*task);        
    //}

    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("transit");
        task->AddDependency("clean");
        task->AddDependency("g-pass");
        task->AddDependency("deferred");
        task->AddDependency("skybox");
        task->AddDependency("fxaa");
        task->AddDependency("armature");

        dx12::CommandList& commandList = *task->GetCommandLists().front();

        commandList.TransitionBarrier(_gBuffer.GetDepthTexture(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
        commandList.TransitionBarrier(_gBuffer.GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList.TransitionBarrier(_gBuffer.GetNormalTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        commandList.Close();
    }

    //if (_renderAABB)
    //{
    //    TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_OBBpipeline);
    //    task->SetName("aabb");
    //    task->AddDependency("clean");
    //    task->AddDependency("g-pass");
    //    task->AddDependency("deferred");
    //    task->AddDependency("skybox");
    //    task->AddDependency("fxaa");
    //    task->AddDependency("armature");
    //    task->AddDependency("transit");

    //    RenderAABB(*task);
    //}

    //GUI
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("gui");
        task->AddDependency("clean");
        task->AddDependency("g-pass");
        task->AddDependency("deferred");
        task->AddDependency("skybox");
        task->AddDependency("fxaa");
        task->AddDependency("armature");
        task->AddDependency("transit");
        task->AddDependency("aabb");

        RenderGUI(*task);
    }

    // Present
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("present");
        task->AddDependency("clean");
        task->AddDependency("g-pass");
        task->AddDependency("deferred");
        task->AddDependency("skybox");
        task->AddDependency("fxaa");
        task->AddDependency("armature");
        task->AddDependency("transit");
        task->AddDependency("aabb");
        task->AddDependency("gui");

        Present(*task);
    }
}

void DXRenderer::OnKeyPressed(Events::KeyEvent& e)
{
    XMVECTOR dir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    if (e.keyCode == DIKeyCode::DIK_W)
    {
        dir += _camera.Look() * _deltaTime * MOVE_SPEED;
    }
    if (e.keyCode == DIKeyCode::DIK_S)
    {
        dir -= _camera.Look() * _deltaTime * MOVE_SPEED;
    }
    if (e.keyCode == DIKeyCode::DIK_D)
    {
        dir += _camera.Right() * _deltaTime * MOVE_SPEED;
    }
    if (e.keyCode == DIKeyCode::DIK_A)
    {
        dir -= _camera.Right() * _deltaTime * MOVE_SPEED;
    }
    _camera.Update(dir);

    switch (e.keyCode)
    {
    case DIKeyCode::DIK_ESCAPE:
        ::SendMessage(_windowHandle, WM_DESTROY, 0, 0);
        break;
    }
}

void DXRenderer::OnMouseMoved(Events::MouseMoveEvent& e)
{
    if ((e.relativeX != 0 || e.relativeY != 0) && _isCameraMoving)
    {
        _camera.Update(e.relativeX, e.relativeY);
    }
}

void DXRenderer::OnMouseButtonPressed(Events::MouseButtonEvent& e)
{
    if (e.rightButton)
    {
        _isCameraMoving = true;
    }
}

void DXRenderer::OnMouseButtonReleased(Events::MouseButtonEvent& e)
{
    if (!e.rightButton)
    {
        _isCameraMoving = false;
    }
}

void DXRenderer::OnResize(Core::Events::ResizeEvent& e)
{
    Frame* current = _currentFrame;
    do
    {
        current->WaitCPU();
        current->ResetGPU();
        current = current->Next;
    } while (current != _currentFrame);

    DirectX::XMUINT2 windowSize = { (uint32_t)e.width, (uint32_t)e.height };

    do
    {
        current->Resize(windowSize);
        current = current->Next;
    } while (current != _currentFrame);

    dx12::Device::OnResize(windowSize);
    _camera.GetViewport().SetSize(windowSize);
    _camera.Update();
    _gBuffer.Init(windowSize);
}

void DXRenderer::ClearBuffers(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();
    commandList.SetName("ClearBuffers");

    dx12::DescriptorHeap& RTVHeap = _currentFrame->GetDescriptorHeap(dx12::DescriptorHeapType::RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = RTVHeap.GetResourceCPUHandle(&_currentFrame->GetTargetTexture());
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = _gBuffer.GetDepthTextureCPUHandle();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Clean");
    {
        commandList.TransitionBarrier(_currentFrame->GetTargetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        commandList.ClearRTV(rtv, clearColor);
        commandList.ClearDSV(dsv, D3D12_CLEAR_FLAG_DEPTH);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::GeometryPass(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();
    commandList.SetName("GeometryPass");

    D3D12_CPU_DESCRIPTOR_HANDLE albedoMetalnessHandle = _gBuffer.GetAlbedoMetalnessTextureCPUHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE normalSpecularHandle = _gBuffer.GetNormalTextureCPUHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = _gBuffer.GetDepthTextureCPUHandle();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Geometry Pass");
    {
        _gBuffer.ClearTextures(commandList);

        commandList.SetPipelineState(_gPassPipeline);

        commandList.SetViewport(_camera.GetViewport());
        commandList.SetRenderTargets({ albedoMetalnessHandle, normalSpecularHandle }, &depthHandle);

#if defined(_DEBUG)
        DebugInfo::StartStatCollecting(commandList);
#endif

        _cachedDataProcessor.Process(_scene, commandList);
        _drawProcessor.Process(_scene, commandList);

#if defined(_DEBUG)
        DebugInfo::EndStatCollecting(commandList);
#endif

        commandList.TransitionBarrier(_gBuffer.GetDepthTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        commandList.TransitionBarrier(_gBuffer.GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        commandList.TransitionBarrier(_gBuffer.GetNormalTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::LightingPass(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();
    commandList.SetName("LightingPass");

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 4, "Deferred Shading");
    {
        commandList.SetPipelineState(_deferredPipeline);

        _cachedDataProcessor.Process(_scene, commandList);

        dx12::DescriptorHeap& RTVHeap = _currentFrame->GetDescriptorHeap(dx12::DescriptorHeapType::RTV);
        dx12::DescriptorHeap& buffersHeap = _currentFrame->GetDescriptorHeap(dx12::DescriptorHeapType::CBV_SRV_UAV);

        dx12::Resource* target = &_currentFrame->GetTargetTexture();
        dx12::Resource* albedoMetalness = &_gBuffer.GetAlbedoMetalnessTexture();
        dx12::Resource* normalSpecular = &_gBuffer.GetNormalTexture();
        dx12::Resource* depth = &_gBuffer.GetDepthTexture();

        dx12::Device::CreateShaderResourceView(albedoMetalness->GetAsSRV(), buffersHeap);
        dx12::Device::CreateShaderResourceView(normalSpecular->GetAsSRV(), buffersHeap);
        dx12::Device::CreateShaderResourceView(depth->GetAsSRV(), buffersHeap);
        dx12::Device::CreateUnorderedAccessView(target->GetAsUAV(), buffersHeap);

        _currentFrame->BindDescriptorHeaps(commandList);

        commandList.SetDescriptorTable(3, buffersHeap.GetResourceGPUHandle(depth));
        commandList.SetDescriptorTable(4, buffersHeap.GetResourceGPUHandle(albedoMetalness));
        commandList.SetDescriptorTable(5, buffersHeap.GetResourceGPUHandle(normalSpecular));
        commandList.SetDescriptorTable(6, buffersHeap.GetResourceGPUHandle(target));

        DirectX::XMUINT2 viewportSize = _camera.GetViewport().GetSize();
        int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
        int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

        commandList.GetDXCommandList()->Dispatch(xThreadGroups, yThreadGroups, 1);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::RenderSkybox(TaskGPU& task)
{
    std::shared_ptr<SceneLayer::Entity> entity = _scene.FindNodeByComponentName("Skybox");
    if (!entity)
    {
        return;
    }

    Skybox* skybox = entity->GetComponentAs<Skybox>("Skybox");

    dx12::CommandList& commandList = *task.GetCommandLists().front();
    commandList.SetName("RenderSkybox");

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "Skybox");
    {
        commandList.SetPipelineState(_SkyboxPipeline);

        _cachedDataProcessor.Process(_scene, commandList);

        dx12::DescriptorHeap& buffersHeap = _currentFrame->GetDescriptorHeap(dx12::DescriptorHeapType::CBV_SRV_UAV);

        dx12::Resource* target = &_currentFrame->GetTargetTexture();
        dx12::Resource* skyboxTexture = skybox->SkydomeTexture.get();
        dx12::Resource* depth = &_gBuffer.GetDepthTexture();

        dx12::Device::CreateShaderResourceView(skyboxTexture->GetAsSRV(), buffersHeap);

        _currentFrame->BindDescriptorHeaps(commandList);

        commandList.SetDescriptorTable(3, buffersHeap.GetResourceGPUHandle(depth));
        commandList.SetDescriptorTable(4, buffersHeap.GetResourceGPUHandle(skyboxTexture));
        commandList.SetDescriptorTable(5, buffersHeap.GetResourceGPUHandle(target));

        DirectX::XMUINT2 viewportSize = _camera.GetViewport().GetSize();
        int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
        int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

        commandList.GetDXCommandList()->Dispatch(xThreadGroups, yThreadGroups, 1);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}
//
//void DXRenderer::RenderFXAA(TaskGPU& task)
//{
//    dx12::CommandList& commandList = *task.GetCommandLists().front();
//
//    PIXBeginEvent(commandList.GetDXCommandList().Get(), 10, "FXAA");
//    {
//        commandList.SetPipelineState(_FXAAPipeline);
//        commandList.SetRootSignature(_FXAAPipeline);
//
//        _cachedDataProcessor.Process(_scene, commandList);
//
//        {
//            ID3D12DescriptorHeap* heap[1] = { _currentFrame->_fxaaHeap.GetDXDescriptorHeap().Get() };
//            commandList.GetDXCommandList()->SetDescriptorHeaps(1, heap);
//
//            D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = _currentFrame->_fxaaHeap.GetResourceGPUHandle(&_currentFrame->_targetTexture);
//            commandList.GetDXCommandList()->SetComputeRootDescriptorTable(3, gpuHandle);
//
//            gpuHandle = _currentFrame->_fxaaHeap.GetResourceGPUHandle(&_currentFrame->_fxaaTexture);
//            commandList.GetDXCommandList()->SetComputeRootDescriptorTable(4, gpuHandle);
//        }
//
//        DirectX::XMUINT2 viewportSize = _camera.GetViewport().GetSize();
//        int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
//        int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);
//
//        commandList.GetDXCommandList()->Dispatch(xThreadGroups, yThreadGroups, 1);
//    }
//    PIXEndEvent(commandList.GetDXCommandList().Get());
//
//    commandList.Close();
//}
//
//void DXRenderer::RenderArmature(TaskGPU& task)
//{
//    dx12::CommandList& commandList = *task.GetCommandLists().front();
//
//    PIXBeginEvent(commandList.GetDXCommandList().Get(), 8, "Armature");
//    {
//        commandList.SetPipelineState(_ArmatureDebugPipeline);
//        commandList.SetRootSignature(_ArmatureDebugPipeline);
//
//        for (auto& node : _scene.GetRootNodes())
//        {
//            Armature* arm = node->GetComponentAs<Armature>("Armature");
//            Transformation* transform = node->GetComponentAs<Transformation>("Transformation");
//
//            if (arm)
//            {
//                DirectX::XMVECTOR* data = (DirectX::XMVECTOR*)arm->BoneDebugTransforms.Map();
//
//                D3D12_CPU_DESCRIPTOR_HANDLE rtv = _currentFrame->_targetHeap->GetCPUDescriptorHandleForHeapStart();
//                D3D12_CPU_DESCRIPTOR_HANDLE dsv = _currentFrame->_depthHeap->GetCPUDescriptorHandleForHeapStart();
//
//                commandList.SetViewport(_camera.GetViewport());
//                commandList.SetRenderTarget(&rtv, &dsv);
//
//                DirectX::XMMATRIX vp = _camera.ViewProjection();
//
//                const auto& sortedBones = arm->GetSortedBones();
//                int ind = 0;
//                for (const auto& bone : sortedBones)
//                {
//                    for (const auto& child : bone->Children)
//                    {
//                        data[0] = DirectX::XMVector4Transform(bone->GlobalTransform.r[3], transform->Transform);
//                        data[1] = DirectX::XMVector4Transform(child->GlobalTransform.r[3], transform->Transform);
//
//                        commandList.SetConstants(0, 16, &vp);
//                        commandList.SetConstants(1, 4, &data[0]);
//                        commandList.SetConstants(1, 4, &data[1], 4);
//                        commandList.SetSRV(2, arm->BoneDebugTransforms.OffsetGPU(0));
//
//                        commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
//
//                        commandList.Draw(1);
//                    }
//                }
//            }
//        }
//    }
//    PIXEndEvent(commandList.GetDXCommandList().Get());
//
//    commandList.Close();
//}
//
//void DXRenderer::RenderAABB(TaskGPU& task)
//{
//    dx12::CommandList& commandList = *task.GetCommandLists().front();
//
//    PIXBeginEvent(commandList.GetDXCommandList().Get(), 8, "AABB");
//    {
//        commandList.SetPipelineState(_OBBpipeline);
//        commandList.SetRootSignature(_OBBpipeline);
//
//        std::vector<SceneLayer::OBBVolume> volumes;
//
//        for (auto& node : _scene.GetRootNodes())
//        {
//            Armature* arm = node->GetComponentAs<Armature>("Armature");
//            Transformation* transform = node->GetComponentAs<Transformation>("Transformation");
//
//            if (arm)
//            {
//                for (const auto& bone : arm->GetSortedBones())
//                {
//                    DirectX::XMMATRIX boneOBB = bone->AABB.Bounds;
//                    boneOBB *= bone->Offset * bone->GlobalTransform * transform->Transform;
//
//                    SceneLayer::OBBVolume obb;
//                    obb.Bounds = boneOBB;
//
//                    volumes.push_back(obb);
//                }
//            }
//        }
//
//        D3D12_CPU_DESCRIPTOR_HANDLE rtv = _currentFrame->_targetHeap->GetCPUDescriptorHandleForHeapStart();
//        D3D12_CPU_DESCRIPTOR_HANDLE dsv = _currentFrame->_depthHeap->GetCPUDescriptorHandleForHeapStart();
//
//        commandList.SetViewport(_camera.GetViewport());
//        commandList.SetRenderTarget(&rtv, &dsv);
//
//        DirectX::XMMATRIX vp = _camera.ViewProjection();
//
//        SceneLayer::AABBVolume aabb = CombineOBBs(volumes);
//
//        DirectX::XMVECTOR center = (aabb.Max - aabb.Min) * 0.5f;
//        DirectX::XMVECTOR translation = (aabb.Max + aabb.Min) * 0.5f;
//        DirectX::XMMATRIX obb = DirectX::XMMatrixScalingFromVector(center) * DirectX::XMMatrixTranslationFromVector(translation);
//
//        commandList.SetConstants(0, 16, &vp);
//        commandList.SetConstants(1, 16, &obb);
//
//        commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
//
//        commandList.Draw(1);
//    }
//    PIXEndEvent(commandList.GetDXCommandList().Get());
//
//    commandList.Close();
//}

void DXRenderer::RenderGUI(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();
    commandList.SetName("RenderGUI");

    dx12::DescriptorHeap& RTVHeap = _currentFrame->GetDescriptorHeap(dx12::DescriptorHeapType::RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = RTVHeap.GetResourceCPUHandle(&_currentFrame->GetTargetTexture());
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = _gBuffer.GetDepthTextureCPUHandle();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "GUI");
    {
        commandList.SetViewport(_camera.GetViewport());
        commandList.SetRenderTarget(&rtv, &dsv);

        if (ImGui::Begin("Debug Info"), true, ImGuiWindowFlags_AlwaysAutoResize)
        {
            ImGui::SetWindowPos({ 0, 0 });
            ImGui::SetWindowSize({ 0, 0 });

            ImGui::Text("FPS: %i (%.03f ms)", DebugInfo::GetFPS(), DebugInfo::GetMsPerFrame());

            if (ImGui::CollapsingHeader("Pipeline statistics"))
            {
                D3D12_QUERY_DATA_PIPELINE_STATISTICS stats = DebugInfo::GetPipelineStatisctics();
                ImGui::Text(std::string("Primitives: " + std::to_string(stats.IAPrimitives)).c_str());
                ImGui::Text(std::string("VS invocs: " + std::to_string(stats.VSInvocations)).c_str());
                ImGui::Text(std::string("GS invocs: " + std::to_string(stats.GSInvocations)).c_str());
                ImGui::Text(std::string("PS invocs: " + std::to_string(stats.PSInvocations)).c_str());
            }

            if (ImGui::CollapsingHeader("Settings"))
            {
                ImGui::SliderFloat("Time multiplier", &_timeMiltiplier, 0.1f, 2.0f, "%.1f");
                ImGui::Checkbox("Render debug armature", &_renderArmature);
                ImGui::Checkbox("Render debug AABB", &_renderAABB);
                ImGui::Checkbox("Render skybox", &_renderSkybox);
                ImGui::Checkbox("Apply FXAA", &_applyFXAA);
            }

            if (ImGui::CollapsingHeader("Inputs"))
            {
                ImGuiIO& io = ImGui::GetIO();
                if (ImGui::IsMousePosValid())
                {
                    ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
                }
                else
                {
                    ImGui::Text("Mouse pos: <INVALID>");
                }
                ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
                ImGui::Text("Mouse down:");
                for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
                {
                    if (ImGui::IsMouseDown(i))
                    {
                        ImGui::SameLine();
                        ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]);
                    }
                }

                struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key >= 0 && key < 512 && ImGui::GetIO().KeyMap[key] != -1; } }; // Hide Native<>ImGuiKey duplicates when both exists in the array
                ImGuiKey start_key = (ImGuiKey)0;

                ImGui::Text("Keys down:");         for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) continue; ImGui::SameLine(); ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key); }
                ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
                ImGui::Text("Chars queue:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
            }
        }
        ImGui::End();

        GUI::Render(commandList);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::Present(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();
    commandList.SetName("Present");

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 6, "Present");
    {
        dx12::Resource& swapChainTexture = *dx12::Device::GetBackBuffer();
        commandList.TransitionBarrier(swapChainTexture, D3D12_RESOURCE_STATE_COPY_DEST);
        commandList.TransitionBarrier(_currentFrame->GetTargetTexture(), D3D12_RESOURCE_STATE_COPY_SOURCE);
        commandList.CopyResource(_currentFrame->GetTargetTexture(), swapChainTexture);
        commandList.TransitionBarrier(swapChainTexture, D3D12_RESOURCE_STATE_PRESENT);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}
