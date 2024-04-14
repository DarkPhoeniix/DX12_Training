#include "stdafx.h"

#include "DXRenderer.h"

#include "DXObjects/Device.h"
#include "Events/MouseScrollEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/RenderEvent.h"
#include "Events/ResizeEvent.h"
#include "Events/UpdateEvent.h"
#include "Events/KeyEvent.h"
#include "Render/TaskGPU.h"

using namespace DirectX;
using namespace Core;

namespace
{
    struct Ambient
    {
        XMFLOAT4 Up;
        XMFLOAT4 Down;
    };
}

DXRenderer::DXRenderer(HWND windowHandle)
    : _windowHandle(windowHandle)
    , _contentLoaded(false)
    , _ambient(nullptr)
    , _DXDevice(Device::GetDXDevice())
{   }

DXRenderer::~DXRenderer()
{
}

bool DXRenderer::LoadContent(TaskGPU* loadTask)
{
    _pipeline.Parse(_DXDevice.Get(), "Resources\\TriangleRenderPipeline.tech");
    _AABBpipeline.Parse(_DXDevice.Get(), "Resources\\AABBRenderPipeline.tech");

    // Camera Setup
    {
        XMVECTOR pos = XMVectorSet(-30.0f, 40.0f, -50.0f, 1.0f);
        XMVECTOR target = XMVectorSet(0.0f, 10.0f, 0.0f, 1.0f);
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        RECT windowSize;
        GetWindowRect(_windowHandle, &windowSize);
        float width = windowSize.right - windowSize.left;
        float height = windowSize.bottom - windowSize.top;
        _camera.SetViewport(Viewport({ width, height }));

        _camera.LookAt(pos, target, up);
        _camera.SetLens(45.0f, 0.1f, 1000.0f);
    }

    // Setup semi-ambient light parameters
    {
        EResourceType CBVType = EResourceType::Dynamic | EResourceType::Buffer | EResourceType::StrideAlignment;

        ResourceDescription desc;
        desc.SetResourceType(CBVType);
        desc.SetSize({ sizeof(Ambient), 1 });
        desc.SetStride(1);
        desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
        _ambient = new Resource(desc);
        _ambient->SetDevice(_DXDevice);
        _ambient->CreateCommitedResource();
        _ambient->SetName("_ambient");

        Ambient* val = (Ambient*)_ambient->Map();
        val->Up = { 0.0f, 0.8f, 0.7f, 1.0f };
        val->Down = { 0.3f, 0.0f, 0.3f, 1.0f };
    }

    // Load scene
    {
        loadTask->SetName("Upload Data");
        auto commandList = loadTask->GetCommandLists().front();

        _scene.LoadScene("dog_t.fbx", commandList);

        DescriptorHeapDescription desc;
        desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        desc.SetNumDescriptors(1);
        desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        desc.SetNodeMask(1);
        _texDescHeap.SetDescription(desc);
        _texDescHeap.SetDevice(_DXDevice);
        _texDescHeap.Create("Textures descriptor heap");

        _tex = Texture::LoadFromFile("dog_tex.dds");
        _tex->SetDXDevice(_DXDevice);
        _tex->SetDescriptorHeap(&_texDescHeap);
        _tex->UploadToGPU(commandList);

        commandList->Close();

        std::vector<ID3D12CommandList*> comLists;
        comLists.reserve(loadTask->GetCommandLists().size());
        for (auto cl : loadTask->GetCommandLists())
        {
            comLists.push_back(cl.Get());
        }

        loadTask->GetCommandQueue()->ExecuteCommandLists(comLists.size(), comLists.data());
        loadTask->GetCommandQueue()->Signal(loadTask->GetFence()->GetFence().Get(), loadTask->GetFenceValue());
    }

    _contentLoaded = true;
    return _contentLoaded;
}

void DXRenderer::OnResize(Events::ResizeEvent& e)
{
    // TODO: not implemented
    //if (e.width != super::getWidth() || e.height != getHeight())
    //{
    //    super::onResize(e);

    //    _viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
    //        static_cast<float>(e.width), static_cast<float>(e.height));

    //    resizeDepthBuffer(e.width, e.height);
    //}
}

void DXRenderer::UnloadContent()
{
    if (_contentLoaded)
    {
        delete _ambient;
        _ambient = nullptr;
    }

    _contentLoaded = false;
}

void DXRenderer::OnUpdate(Events::UpdateEvent& updateEvent)
{
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;

    //super::OnUpdate(updateEvent);

    totalTime += updateEvent.elapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        int fps = frameCount / totalTime;

        std::wstring fpsText = L"FPS: " + std::to_wstring(fps);
        ::SetWindowText(_windowHandle, fpsText.c_str());

        frameCount = 0;
        totalTime = 0.0;
    }
}

// Transition a resource
void DXRenderer::transitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
    ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

void DXRenderer::OnRender(Events::RenderEvent& renderEvent, Frame& frame)
{
    frame.WaitCPU();
    frame.ResetGPU();

    auto rtv = frame._targetHeap->GetCPUDescriptorHandleForHeapStart();
    auto dsv = frame._depthHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear render targets
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("clean");

        ComPtr<ID3D12GraphicsCommandList2> commandList = task->GetCommandLists().front();

        transitionResource(commandList, frame._targetTexture.GetDXResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
        commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        commandList->Close();
    }

    // Execute the TriangleRender shader
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, _pipeline.GetPipelineState());
        task->SetName("render");
        task->AddDependency("clean");

        ComPtr<ID3D12GraphicsCommandList2> commandList = task->GetCommandLists().front();

        commandList->SetPipelineState(_pipeline.GetPipelineState().Get());
        commandList->SetGraphicsRootSignature(_pipeline.GetRootSignature().Get());

        CD3DX12_VIEWPORT viewport = _camera.GetDXViewport();
        CD3DX12_RECT scissorRect = _camera.GetDXScissorRectangle();
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

        XMMATRIX viewProjMatrix = XMMatrixMultiply(_camera.View(), _camera.Projection());
        commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &viewProjMatrix, 0);
        commandList->SetGraphicsRootConstantBufferView(1, _ambient->OffsetGPU(0));

        ID3D12DescriptorHeap* heaps = { _texDescHeap.GetDXDescriptorHeap().Get()};
        commandList->SetDescriptorHeaps(1, &heaps);

        commandList->SetGraphicsRootDescriptorTable(3, _texDescHeap.GetResourceGPUHandle(_tex.get()));

        _scene.Draw(commandList, _camera.GetViewFrustum());

        commandList->SetPipelineState(_AABBpipeline.GetPipelineState().Get());
        commandList->SetGraphicsRootSignature(_AABBpipeline.GetRootSignature().Get());

        commandList->SetGraphicsRoot32BitConstants(1, sizeof(XMMATRIX) / 4, &viewProjMatrix, 0);

        _scene.DrawAABB(commandList);

        commandList->Close();
    }

    // Present
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("present");
        task->AddDependency("render");

        ComPtr<ID3D12GraphicsCommandList2> commandList = task->GetCommandLists().front();

        transitionResource(commandList, frame._swapChainTexture.GetDXResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
        transitionResource(commandList, frame._targetTexture.GetDXResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

        commandList->CopyResource(frame._swapChainTexture.GetDXResource().Get(), frame._targetTexture.GetDXResource().Get());

        transitionResource(commandList, frame._swapChainTexture.GetDXResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);

        commandList->Close();
    }
}

void DXRenderer::OnKeyPressed(Events::KeyEvent& e)
{
    XMVECTOR dir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    if (e.keyCode == DIKeyCode::DIK_W)
    {
        dir += _camera.Look();
    }
    if (e.keyCode == DIKeyCode::DIK_S)
    {
        dir -= _camera.Look();
    }
    if (e.keyCode == DIKeyCode::DIK_D)
    {
        dir += _camera.Right();
    }
    if (e.keyCode == DIKeyCode::DIK_A)
    {
        dir -= _camera.Right();
    }
    _camera.Update(dir);

    //switch (e.key)
    //{
    //case KeyCode::Escape:
    //    //Application::Get().Quit(0);
    //    break;
    //case KeyCode::Enter:        // TODO: looks weird
    //    if (e.alt)
    //    {
    //case KeyCode::F11:
    //    //_window->toggleFullscreen();
    //    break;
    //    }
    //case KeyCode::V:
    //    //_window->toggleVSync();
    //    break;
    //case KeyCode::Space:
    //    break;
    //}
}

void DXRenderer::OnMouseScroll(Events::MouseScrollEvent& e)
{
    //_FoV -= e.scrollDelta;
    //_FoV = clamp(_FoV, 12.0f, 90.0f);
    //camera.SetFOV(_FoV);

    //char buffer[256];
    //sprintf_s(buffer, "FoV: %f\n", _FoV);
    //OutputDebugStringA(buffer);
}

void DXRenderer::OnMouseMoved(Events::MouseMoveEvent& e)
{
    if ((e.relativeX != 0 || e.relativeY != 0) && _isCameraMoving)
        _camera.Update(e.relativeX, e.relativeY);
}

void DXRenderer::OnMouseButtonPressed(Events::MouseButtonEvent& e)
{
    if (e.button == Events::MouseButtonEvent::MouseButton::Right)
        _isCameraMoving = true;
}

void DXRenderer::OnMouseButtonReleased(Events::MouseButtonEvent& e)
{
    if (e.button == Events::MouseButtonEvent::MouseButton::Right)
        _isCameraMoving = false;
}
