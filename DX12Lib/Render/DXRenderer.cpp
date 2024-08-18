#include "stdafx.h"

#include "DXRenderer.h"

#include "DXObjects/GraphicsCommandList.h"
#include "Events/MouseScrollEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/RenderEvent.h"
#include "Events/ResizeEvent.h"
#include "Events/UpdateEvent.h"
#include "Events/KeyEvent.h"
#include "Render/TaskGPU.h"

#include "GUI/GUI.h"

using namespace DirectX;
using namespace Core;

namespace
{
    constexpr float MOVE_SPEED = 200.0f;

    struct Ambient
    {
        XMFLOAT4 Up;
        XMFLOAT4 Down;
    };
}

DXRenderer::DXRenderer(HWND windowHandle)
    : _DXDevice(Device::GetDXDevice())
    , _windowHandle(windowHandle)
    , _contentLoaded(false)
    , _ambient(nullptr)
    , _isCameraMoving(false)
    , _deltaTime(0.0f)
    , _light(nullptr)
{   }

DXRenderer::~DXRenderer()
{
    _DXDevice = nullptr;
}

bool DXRenderer::LoadContent(TaskGPU* loadTask)
{
    _renderPipeline.Parse("PipelineDescriptions\\TriangleRenderPipeline.tech");
    _AABBpipeline.Parse("PipelineDescriptions\\AABBRenderPipeline.tech");


    // TODO: rework
    {
        ComPtr<ID3DBlob> computeShaderBlob = nullptr;
        Helper::throwIfFailed(D3DReadFileToBlob(L"NegativePostFX_cs.cso", &computeShaderBlob));

        D3D12_INPUT_ELEMENT_DESC* inputLayout = nullptr;

        D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateStreamDesc = {};

        Helper::throwIfFailed(_DXDevice->CreateRootSignature(0, computeShaderBlob->GetBufferPointer(),
            computeShaderBlob->GetBufferSize(), IID_PPV_ARGS(&_postFXRootSig)));

        pipelineStateStreamDesc.CS = CD3DX12_SHADER_BYTECODE(computeShaderBlob.Get());

        Helper::throwIfFailed(_DXDevice->CreateComputePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&_postFXPipeState)));
    }


#if defined(_DEBUG)
    _statsQuery.Create();
#endif

    // Camera Setup
    {
        XMVECTOR pos = XMVectorSet(-30.0f, 40.0f, -50.0f, 1.0f);
        XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
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
        desc.SetSize({ sizeof(DirectionalLight), 1 });
        desc.SetStride(1);
        desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
        _light = std::make_shared<Resource>(desc);
        _light->CreateCommitedResource();
        _light->SetName("_light");

        DirectionalLight* val = (DirectionalLight*)_light->Map();
        val->SetDirection(XMVectorSet(0.5f, -0.8f, 0.3f, 1.0f));
        val->SetColor(XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // Load scene
    {
        loadTask->SetName("Upload Data");
        Core::GraphicsCommandList* commandList = loadTask->GetCommandLists().front();

        _scene.LoadScene("Wyvern\\Wyvern.scene", *commandList);

        commandList->Close();

        std::vector<ID3D12CommandList*> comLists;
        comLists.reserve(loadTask->GetCommandLists().size());
        for (auto cl : loadTask->GetCommandLists())
        {
            comLists.push_back(cl->GetDXCommandList().Get());
        }

        loadTask->GetCommandQueue()->ExecuteCommandLists(comLists.size(), comLists.data());
        loadTask->GetCommandQueue()->Signal(loadTask->GetFence()->GetFence().Get(), loadTask->GetFenceValue());
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
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;
    static double totalTimeStat = 0.0;

    totalTime += updateEvent.elapsedTime;
    totalTimeStat += updateEvent.elapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        int fps = (int)(frameCount / totalTime);

        std::wstring fpsText = L"FPS: " + std::to_wstring(fps);
        ::SetWindowText(_windowHandle, fpsText.c_str());

        frameCount = 0;
        totalTime = 0.0;
    }

    if (totalTimeStat > 5.0)
    {
        totalTimeStat = 0.0;

#if defined(_DEBUG)
        D3D12_QUERY_DATA_PIPELINE_STATISTICS stat = _statsQuery.GetStatistics();
        std::string d = "Primitives rendered: " + std::to_string(stat.IAPrimitives) + "\n";
        OutputDebugStringA(d.c_str());
        d = "VS invocs: " + std::to_string(stat.VSInvocations) + "\n";
        OutputDebugStringA(d.c_str());
        d = "PS invocs: " + std::to_string(stat.PSInvocations) + "\n";
        OutputDebugStringA(d.c_str());
#endif
    }

    _deltaTime = updateEvent.elapsedTime;
}

void DXRenderer::OnRender(Events::RenderEvent& renderEvent, Frame& frame)
{
    frame.WaitCPU();
    frame.ResetGPU();

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = frame._targetHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = frame._depthHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear render targets
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("clean");

        Core::GraphicsCommandList* commandList = task->GetCommandLists().front();
        PIXBeginEvent(commandList->GetDXCommandList().Get(), 1, "Clean");

        commandList->TransitionBarrier(frame._targetTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        commandList->ClearRTV(rtv, clearColor);
        commandList->ClearDSV(dsv, D3D12_CLEAR_FLAG_DEPTH);

        PIXEndEvent(commandList->GetDXCommandList().Get());
        commandList->Close();
    }

    // Execute the TriangleRender shader
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_renderPipeline);
        task->SetName("render");
        task->AddDependency("clean");

        Core::GraphicsCommandList* commandList = task->GetCommandLists().front();
        PIXBeginEvent(commandList->GetDXCommandList().Get(), 4, "Render");

#if defined(_DEBUG)
        _statsQuery.BeginQuery(*commandList);
#endif
        commandList->SetPipelineState(_renderPipeline);
        commandList->SetGraphicsRootSignature(_renderPipeline);

        commandList->SetViewport(_camera.GetViewport());
        commandList->SetRenderTarget(&rtv, &dsv);

        XMMATRIX viewProjMatrix = XMMatrixMultiply(_camera.View(), _camera.Projection());
        commandList->SetConstants(0, sizeof(XMMATRIX) / 4, &viewProjMatrix);
        
        commandList->SetCBV(2, _light->OffsetGPU(0));

        _scene.Draw(*commandList, _camera.GetViewFrustum());

#if defined(_DEBUG)
        _statsQuery.EndQuery(*commandList);
        _statsQuery.ResolveQueryData(*commandList);

        commandList->SetPipelineState(_AABBpipeline);
        commandList->SetGraphicsRootSignature(_AABBpipeline);
        commandList->SetConstants(1, sizeof(XMMATRIX) / 4, &viewProjMatrix);

        _scene.DrawAABB(*commandList);
#endif

        PIXEndEvent(commandList->GetDXCommandList().Get());
        commandList->Close();
    }

    //{

    //    TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
    //    task->SetName("postfx");
    //    task->AddDependency("render");

    //    Core::GraphicsCommandList* commandList = task->GetCommandLists().front();
    //    PIXBeginEvent(commandList->GetDXCommandList().Get(), 4, "Render");

    //    commandList->GetDXCommandList()->SetPipelineState(_postFXPipeState.Get());
    //    commandList->GetDXCommandList()->SetComputeRootSignature(_postFXRootSig.Get());


    //    commandList->TransitionBarrier(frame._targetTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);


    //    commandList->SetDescriptorHeaps({ frame._postFXDescHeap.GetDXDescriptorHeap().Get() });

    //    commandList->GetDXCommandList()->SetComputeRootDescriptorTable(0, frame._postFXDescHeap.GetHeapStartGPUHandle());

    //    const UINT x = 1280 / 2;
    //    const UINT y = 720 / 2;
    //    commandList->GetDXCommandList()->Dispatch(x, y, 1);

    //    commandList->TransitionBarrier(frame._targetTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

    //    PIXEndEvent(commandList->GetDXCommandList().Get());
    //    commandList->Close();
    //}

    //GUI
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("gui");
        task->AddDependency("render");

        Core::GraphicsCommandList* commandList = task->GetCommandLists().front();
        PIXBeginEvent(commandList->GetDXCommandList().Get(), 4, "GUI");

        commandList->SetViewport(_camera.GetViewport());
        commandList->SetRenderTarget(&rtv, &dsv);

        GUI::Render(*commandList);

        PIXEndEvent(commandList->GetDXCommandList().Get());
        commandList->Close();
    }

    // Present
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("present");
        task->AddDependency("gui");

        Core::GraphicsCommandList* commandList = task->GetCommandLists().front();
        PIXBeginEvent(commandList->GetDXCommandList().Get(), 5, "Present");

        commandList->TransitionBarrier(frame._swapChainTexture, D3D12_RESOURCE_STATE_COPY_DEST);
        commandList->TransitionBarrier(frame._targetTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
        commandList->CopyResource(frame._targetTexture, frame._swapChainTexture);
        commandList->TransitionBarrier(frame._swapChainTexture, D3D12_RESOURCE_STATE_PRESENT);

        PIXEndEvent(commandList->GetDXCommandList().Get());
        commandList->Close();
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
