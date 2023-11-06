
#include "pch.h"

#include "RenderCubeExample.h"

#include "Application.h"
#include "CommandQueue.h"
#include "Events/KeyEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/MouseScrollEvent.h"
#include "Events/RenderEvent.h"
#include "Events/ResizeEvent.h"
#include "Events/UpdateEvent.h"

#include <random>
#include <cmath>

using namespace DirectX;

namespace
{
    // TODO: remove
    static float deltaTime = 0.0f;

    const FLOAT BACKGROUND_COLOR[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Clamp a value between a min and max range.
    template<typename T>
    constexpr const T& clamp(const T& val, const T& min, const T& max)
    {
        return (val < min) ? min : (val > max) ? max : val;
    }

    // Vertex data for a colored cube.
    struct VertexPosColor
    {
        XMFLOAT3 Position;
        XMFLOAT3 Color;
    };

    const std::vector<VertexPosColor> CUBE_VERTICES = {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },  // 0
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },  // 1
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },   // 2
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },   // 3
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },  // 4
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },  // 5
        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },   // 6
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }    // 7
    };

    const std::vector<WORD> CUBE_INDICES =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };
}

RenderCubeExample::RenderCubeExample(const std::wstring& name, int width, int height, bool vSync)
    : super(name, width, height, vSync)
    , _scissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , _viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
    , _FoV(45.0)
    , _contentLoaded(false)
    , distribution({-180.0f, -100.0f, 0.0f, 1.0f}, { 180.0f, 100.0f, 0.0f, 1.0f }, 10, 20)
{
    distribution.Init();
    XMVECTOR pos = XMVectorSet(0.0f, 0.0f, -20.0f, 1.0f);
    XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    camera.LookAt(pos, target, up);
    float aspectRatio = getWidth() / static_cast<float>(getHeight());
    camera.SetLens(45.0f, aspectRatio, 0.1f, 1000.0f);
}

void RenderCubeExample::updateBufferResource(
    ComPtr<ID3D12GraphicsCommandList2> commandList,
    ID3D12Resource** destinationResource,
    ID3D12Resource** intermediateResource,
    size_t numElements, size_t elementSize, const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::get().getDevice();

    size_t bufferSize = numElements * elementSize;

    CD3DX12_HEAP_PROPERTIES heapTypeDefault(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferWithFlags = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    Helper::throwIfFailed(device->CreateCommittedResource(
        &heapTypeDefault,
        D3D12_HEAP_FLAG_NONE,
        &bufferWithFlags,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(destinationResource)));

    CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC buffer = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    if (bufferData)
    {
        Helper::throwIfFailed(device->CreateCommittedResource(
            &heapTypeUpload,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(intermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(),
            *destinationResource, *intermediateResource,
            0, 0, 1, &subresourceData);
    }
}


bool RenderCubeExample::loadContent()
{
    auto device = Application::get().getDevice();
    auto commandQueue = Application::get().getCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList = commandQueue->getCommandList();

    // Upload vertex buffer data.
    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    updateBufferResource(commandList,
        &_vertexBuffer, &intermediateVertexBuffer,
        CUBE_VERTICES.size(), sizeof(VertexPosColor), CUBE_VERTICES.data());

    // Create the vertex buffer view.
    _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
    _vertexBufferView.SizeInBytes = static_cast<UINT>(CUBE_VERTICES.size() * sizeof(VertexPosColor));
    _vertexBufferView.StrideInBytes = sizeof(VertexPosColor);

    // Upload index buffer data.
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
    updateBufferResource(commandList,
        &_indexBuffer, &intermediateIndexBuffer,
        CUBE_INDICES.size(), sizeof(WORD), CUBE_INDICES.data());

    // Create index buffer view.
    _indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
    _indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    _indexBufferView.SizeInBytes = static_cast<UINT>(CUBE_INDICES.size() * sizeof(WORD));

    // Create the descriptor heap for the depth-stencil view.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Helper::throwIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_DSVHeap)));

    // Load the vertex shader.
    ComPtr<ID3DBlob> vertexShaderBlob;
    Helper::throwIfFailed(D3DReadFileToBlob(L"TriangleVertexShader.cso", &vertexShaderBlob));

    // Load the pixel shader.
    ComPtr<ID3DBlob> pixelShaderBlob;
    Helper::throwIfFailed(D3DReadFileToBlob(L"TrianglePixelShader.cso", &pixelShaderBlob));

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    // Denying access to shader stages that do not require root signature access 
    // is a minor optimization on some hardware.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    Helper::throwIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
    //Helper::throwIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
    //    rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));

     Helper::throwIfFailed(device->CreateRootSignature(0, vertexShaderBlob->GetBufferPointer(),
         vertexShaderBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));


    //struct PipelineStateStream
    //{
    //    CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
    //    CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
    //    CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
    //    CD3DX12_PIPELINE_STATE_STREAM_VS VS;
    //    CD3DX12_PIPELINE_STATE_STREAM_PS PS;
    //    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
    //    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    //} pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateStreamDesc = {};

    D3D12_BLEND_DESC blend = {};
    blend.RenderTarget[0].BlendEnable = true;
    blend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

    blend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    blend.RenderTarget[0].LogicOpEnable = false;
    blend.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;

    D3D12_RASTERIZER_DESC raster = {};
    raster.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
    raster.CullMode = D3D12_CULL_MODE_NONE;
    raster.DepthClipEnable = true;

    D3D12_DEPTH_STENCIL_DESC depth = {};
    depth.DepthEnable = true;
    depth.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depth.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depth.StencilEnable = false;

    pipelineStateStreamDesc.BlendState = blend;
    pipelineStateStreamDesc.RasterizerState = raster;
    pipelineStateStreamDesc.DepthStencilState = depth;

    pipelineStateStreamDesc.pRootSignature = _rootSignature.Get();
    pipelineStateStreamDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStreamDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStreamDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStreamDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStreamDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStreamDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineStateStreamDesc.NumRenderTargets = 1;
    pipelineStateStreamDesc.SampleDesc.Count = 1; // must be the same sample description as the swapchain and depth/stencil buffer
    pipelineStateStreamDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done

    Helper::throwIfFailed(device->CreateGraphicsPipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&_pipelineState)));

    auto fenceValue = commandQueue->executeCommandList(commandList);
    commandQueue->waitForFenceValue(fenceValue);

    _contentLoaded = true;

    // Resize/Create the depth buffer.
    resizeDepthBuffer(getWidth(), getHeight());

    return true;
}

void RenderCubeExample::resizeDepthBuffer(int width, int height)
{
    if (_contentLoaded)
    {
        // Flush any GPU commands that might be referencing the depth buffer.
        Application::get().flush();

        width = std::max(1, width);
        height = std::max(1, height);

        auto device = Application::get().getDevice();

        // Resize screen dependent resources.
        // Create a depth buffer.
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };

        CD3DX12_HEAP_PROPERTIES heapTypeDefault(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC tex2D = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
            1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        Helper::throwIfFailed(device->CreateCommittedResource(
            &heapTypeDefault,
            D3D12_HEAP_FLAG_NONE,
            &tex2D,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&_depthBuffer)
        ));

        // Update the depth-stencil view.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(_depthBuffer.Get(), &dsv,
            _DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void RenderCubeExample::onResize(ResizeEvent& e)
{
    if (e.width != super::getWidth() || e.height != getHeight())
    {
        super::onResize(e);

        _viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
            static_cast<float>(e.width), static_cast<float>(e.height));

        resizeDepthBuffer(e.width, e.height);
    }
}

void RenderCubeExample::unloadContent()
{
    _contentLoaded = false;
}

void RenderCubeExample::onUpdate(UpdateEvent& updateEvent)
{
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;

    super::onUpdate(updateEvent);

    totalTime += updateEvent.elapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        double fps = frameCount / totalTime;

        char buffer[512];
        sprintf_s(buffer, "FPS: %f\n", fps);
        OutputDebugStringA(buffer);

        frameCount = 0;
        totalTime = 0.0;
    }

    // Update the model matrix.
    //float angle = static_cast<float>(updateEvent.totalTime * 90.0f);
    //const XMVECTOR rotationAxis = XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f);
    //_modelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));
    _modelMatrix = XMMatrixIdentity();

    // Update the view matrix.
    const XMVECTOR eyePosition = XMVectorSet(0.0f, 0.0f, 200.0f, 1.0f);
    const XMVECTOR focusPoint = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    const XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    _viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

    // Update the projection matrix.
    float aspectRatio = getWidth() / static_cast<float>(getHeight());
    _projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(_FoV), aspectRatio, 0.1f, 1000.0f);

    distribution.TrySpawnStep();
}

// Transition a resource
void RenderCubeExample::transitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    Microsoft::WRL::ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

// Clear a render target.
void RenderCubeExample::clearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void RenderCubeExample::clearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void RenderCubeExample::onRender(RenderEvent& renderEvent)
{
    super::onRender(renderEvent);

    auto commandQueue = Application::get().getCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue->getCommandList();

    UINT currentBackBufferIndex = _window->getCurrentBackBufferIndex();
    auto backBuffer = _window->getCurrentBackBuffer();
    auto rtv = _window->getCurrentRenderTargetView();
    auto dsv = _DSVHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear the render targets.
    {
        transitionResource(commandList, backBuffer,
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        clearRTV(commandList, rtv, clearColor);
        clearDepth(commandList, dsv);
    }

    commandList->SetPipelineState(_pipelineState.Get());
    commandList->SetGraphicsRootSignature(_rootSignature.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
    commandList->IASetIndexBuffer(&_indexBufferView);

    commandList->RSSetViewports(1, &_viewport);
    commandList->RSSetScissorRects(1, &_scissorRect);

    commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    // Update the MVP matrix
    XMMATRIX mvpMatrix = XMMatrixMultiply(_modelMatrix, camera.View());
    mvpMatrix = XMMatrixMultiply(mvpMatrix, camera.Projection());
    //XMMATRIX mvpMatrix = XMMatrixMultiply(_modelMatrix, _viewMatrix);
    //mvpMatrix = XMMatrixMultiply(mvpMatrix, _projectionMatrix);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

    commandList->DrawIndexedInstanced(static_cast<UINT>(CUBE_INDICES.size()), 1, 0, 0, 0);

    //for (const auto& cube : distribution.GetLocationsArray())
    //{
    //    // Cube reneder
    //    commandList->DrawIndexedInstanced(static_cast<UINT>(CUBE_INDICES.size()), 1, 0, 0, 0);

    //    _modelMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixTranslation(XMVectorGetX(cube), XMVectorGetY(cube), XMVectorGetZ(cube)));
    //    mvpMatrix = XMMatrixMultiply(_modelMatrix, _viewMatrix);
    //    mvpMatrix = XMMatrixMultiply(mvpMatrix, _projectionMatrix);
    //    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);
    //}

    // Present
    {
        transitionResource(commandList, backBuffer,
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        _fenceValues[currentBackBufferIndex] = commandQueue->executeCommandList(commandList);

        currentBackBufferIndex = _window->present();

        commandQueue->waitForFenceValue(_fenceValues[currentBackBufferIndex]);
    }
}

void RenderCubeExample::onKeyPressed(KeyEvent& e)
{
    super::onKeyPressed(e);

    XMVECTOR dir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    if (e.key == KeyCode::W)
    {
        dir += camera.Look();
    }
    if (e.key == KeyCode::S)
    {
        dir -= camera.Look();
    }
    if (e.key == KeyCode::D)
    {
        dir += camera.Right();
    }
    if (e.key == KeyCode::A)
    {
        dir -= camera.Right();
    }
    camera.Update(dir);

    switch (e.key)
    {
    case KeyCode::Escape:
        Application::get().quit(0);
        break;
    case KeyCode::Enter:        // TODO: looks weird
        if (e.alt)
        {
            case KeyCode::F11:
                _window->toggleFullscreen();
                break;
        }
    case KeyCode::V:
        _window->toggleVSync();
        break;
    case KeyCode::Space:
        // TODO: add a Reset for the Poisson Distribution on Space button
        break;
    }
}

void RenderCubeExample::onMouseScroll(MouseScrollEvent& e)
{
    _FoV -= e.scrollDelta;
    _FoV = clamp(_FoV, 12.0f, 90.0f);

    char buffer[256];
    sprintf_s(buffer, "FoV: %f\n", _FoV);
    OutputDebugStringA(buffer);
}

void RenderCubeExample::onMouseMoved(MouseMoveEvent& e)
{
    int pitch = e.relativeX;
    int yAngle = e.relativeY;

    if (std::abs(pitch) > 0 && _isCameraMoving)
        camera.Update(pitch, yAngle);
}

void RenderCubeExample::onMouseButtonPressed(MouseButtonEvent& e)
{
    if (e.button == MouseButtonEvent::MouseButton::Right)
        _isCameraMoving = true;
}

void RenderCubeExample::onMouseButtonReleased(MouseButtonEvent& e)
{
    if (e.button == MouseButtonEvent::MouseButton::Right)
        _isCameraMoving = false;
}
