
#include "stdafx.h"

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
//#include "Utility/PipelineSettingsParser.h"

#include <random>
#include <cmath>

using namespace DirectX;

namespace
{
    //XMFLOAT3 AmbientUp = { 0.78f, 0.22f, 1.0f };
    //XMFLOAT3 AmbientDown = { 1.0f, 0.0f, 0.0f };

    struct Ambient
    {
        XMFLOAT4 Up;
        XMFLOAT4 Down;
    };
    //Ambient amb = { { 0.39f, 0.25f, 1.0f }, { 0.17f, 0.73f, 0.51f } };

    // TODO: remove later
    double DELTATIME = 0.0f;
    const FLOAT BACKGROUND_COLOR[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    const int CUBES_SIZE = 1;

    // Clamp a value between a min and max range.
    template<typename T>
    constexpr const T& clamp(const T& val, const T& min, const T& max)
    {
        return (val < min) ? min : (val > max) ? max : val;
    }

    // Vertex data for a colored cube.
    //struct VertexPosColor
    //{
    //    XMFLOAT3 Position;
    //    XMFLOAT3 Color;
    //};

    const std::vector<VertexPosColor> CUBE_VERTICES = {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },  // 0
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },  // 1
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },   // 2
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },   // 3
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },  // 4
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },  // 5
        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },   // 6
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }    // 7
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
    , _contentLoaded(false)
    , distribution({-20.0f, -20.0f, -20.0f, 1.0f}, { 20.0f, 20.0f, 20.0f, 1.0f }, 10, 20)
    //, _model(CUBE_VERTICES, CUBE_INDICES)
    , _model("cat.obj")
{
    //_model.ParseFile("model.obj");
    distribution.Init();

    XMVECTOR pos = XMVectorSet(0.0f, 0.0f, -100.0f, 1.0f);
    XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    float aspectRatio = getWidth() / static_cast<float>(getHeight());

    camera.LookAt(pos, target, up);
    camera.SetLens(45.0f, aspectRatio, 0.1f, 1000.0f);

    D3D12_HEAP_PROPERTIES heapDesc = {};
    {
        memset(&heapDesc, 0, sizeof(D3D12_HEAP_PROPERTIES));

        heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapDesc.CreationNodeMask = 1;
        heapDesc.VisibleNodeMask = 1;

        heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
    }
    D3D12_HEAP_DESC desc;
    desc.Alignment = 0;
    desc.Flags = D3D12_HEAP_FLAG_NONE;
    desc.Properties = heapDesc;
    desc.SizeInBytes = 15 * _1MB;
    Application::get().getDevice()->CreateHeap(&desc, IID_PPV_ARGS(&_pHeap));

    EResourceType type = EResourceType::Dynamic | EResourceType::Buffer;
    EResourceType type1 = EResourceType::Dynamic | EResourceType::Buffer | EResourceType::StrideAlignment;
    _ambient = new Resource(Helper::CreateBuffers(Application::get().getDevice(), type1, 1, sizeof(Ambient)));
    _ambient->_resource->SetName(L"_ambient");
    Ambient* val = (Ambient*)_ambient->Map();
    //val->Up = { 0.39f, 0.25f, 1.0f };
    //val->Down = { 0.17f, 0.73f, 0.51f };
    val->Up = { 0.0f, 0.8f, 0.7f, 1.0f };
    val->Down = { 0.3f, 0.0f, 0.3f, 1.0f };

    _cubeTransformsRes[0] = new Resource(Helper::CreateBuffers(_pHeap.Get(), Application::get().getDevice(), type, 5 * _1MB, 1, 0));
    _cubeTransformsRes[1] = new Resource(Helper::CreateBuffers(_pHeap.Get(), Application::get().getDevice(), type, 5 * _1MB, 1, 5 * _1MB));
    _cubeTransformsRes[2] = new Resource(Helper::CreateBuffers(_pHeap.Get(), Application::get().getDevice(), type, 5 * _1MB, 1, 10 * _1MB));
    _cubeTransformsRes[0]->_resource->SetName(L"_cubeTransformsRes[0]");
    _cubeTransformsRes[1]->_resource->SetName(L"_cubeTransformsRes[1]");
    _cubeTransformsRes[2]->_resource->SetName(L"_cubeTransformsRes[2]");

    _transfP[0] = (DirectX::XMMATRIX*)_cubeTransformsRes[0]->Map();
    _transfP[1] = (DirectX::XMMATRIX*)_cubeTransformsRes[1]->Map();
    _transfP[2] = (DirectX::XMMATRIX*)_cubeTransformsRes[2]->Map();




    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descHeapDesc.NodeMask = 0;
    descHeapDesc.NumDescriptors = 32;
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    Application::get().getDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_descHeap));


    _dynamicData = new Resource(Helper::CreateBuffers(Application::get().getDevice(), EResourceType::Dynamic | EResourceType::Buffer, 2, sizeof(XMFLOAT4)));
    _UAVRes = new Resource(Helper::CreateBuffers(Application::get().getDevice(), EResourceType::Unordered | EResourceType::Texture, 64,64, sizeof(float) * 4, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    _readBack = new Resource(Helper::CreateBuffers(Application::get().getDevice(), EResourceType::ReadBack | EResourceType::Buffer, 1, sizeof(float)));
    _dynamicData->_resource->SetName(L"_dynamicData");
    _UAVRes->_resource->SetName(L"UAV texture");
    _readBack->_resource->SetName(L"_readBack");

    XMFLOAT4* flData = (XMFLOAT4*)_dynamicData->Map();
    flData[0] = { 1.0f, 0.0f, 0.0f, 1.0f };
    flData[1] = { 0.0f, 0.0f, 1.0f, 1.0f };

    auto  asdas  = Application::get().getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto  asdas1 = Application::get().getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    auto  asdas2 = Application::get().getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    auto  asdas3 = Application::get().getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    D3D12_CPU_DESCRIPTOR_HANDLE offset = _descHeap->GetCPUDescriptorHandleForHeapStart();
    offset.ptr += asdas * 0;

    D3D12_CPU_DESCRIPTOR_HANDLE offset2 = _descHeap->GetCPUDescriptorHandleForHeapStart();
    offset2.ptr += asdas * 1;


    D3D12_UNORDERED_ACCESS_VIEW_DESC _resDesc = {};
    _resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    _resDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    _resDesc.Texture2D.MipSlice = 0;
    Application::get().getDevice()->CreateUnorderedAccessView(_UAVRes->_resource.Get(), nullptr, &_resDesc, offset);



    D3D12_SHADER_RESOURCE_VIEW_DESC _resDesc2 = {};
    _resDesc2.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    _resDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    _resDesc2.Texture2D.MipLevels = 1;
    _resDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    Application::get().getDevice()->CreateShaderResourceView(_UAVRes->_resource.Get(), &_resDesc2, offset2);

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
    (*destinationResource)->GetGPUVirtualAddress();

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

    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    ComPtr<ID3D12Resource> intermediateIndexBuffer;

    // Vertex Buffer Setup
    {
        // Upload vertex buffer data.
        updateBufferResource(commandList,
            &_vertexBuffer, &intermediateVertexBuffer,
            _model.GetVertices().size(), sizeof(VertexPosColor), _model.GetVertices().data());

        // Create the vertex buffer view.
        _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
        _vertexBufferView.SizeInBytes = static_cast<UINT>(_model.GetVertices().size() * sizeof(VertexPosColor));
        _vertexBufferView.StrideInBytes = sizeof(VertexPosColor);
    }

    // Index Buffer Setup
    {
        // Upload index buffer data.
        updateBufferResource(commandList,
            &_indexBuffer, &intermediateIndexBuffer,
            _model.GetIndices().size(), sizeof(WORD), _model.GetIndices().data());

        // Create index buffer view.
        _indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
        _indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        _indexBufferView.SizeInBytes = static_cast<UINT>(_model.GetIndices().size() * sizeof(WORD));
    }

    // Create the descriptor heap for the depth-stencil view.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Helper::throwIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_DSVHeap)));

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipeline.Parse(device.Get(), "RenderPipeline.tech");

    {
        std::string computeShaderFilepath = "Compute.cso";
        ComPtr<ID3DBlob> computeShaderBlob;
        Helper::throwIfFailed(D3DReadFileToBlob(std::wstring(computeShaderFilepath.begin(), computeShaderFilepath.end()).c_str(), &computeShaderBlob));

        Helper::throwIfFailed(device->CreateRootSignature(0, computeShaderBlob->GetBufferPointer(),
            computeShaderBlob->GetBufferSize(), IID_PPV_ARGS(&_rootComputeSignature)));

        D3D12_COMPUTE_PIPELINE_STATE_DESC _compDesc = {};
        _compDesc.NodeMask = 0;
        _compDesc.CS = CD3DX12_SHADER_BYTECODE(computeShaderBlob.Get());
        _compDesc.pRootSignature = _rootComputeSignature.Get();
        _compDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        Helper::throwIfFailed(device->CreateComputePipelineState(&_compDesc, IID_PPV_ARGS(&_pipelineComputeState)));
    }

    auto fenceValue = commandQueue->executeCommandList(commandList);
    commandQueue->waitForFenceValue(fenceValue);

    _contentLoaded = true;

    // Resize/Create the depth buffer.
    resizeDepthBuffer(getWidth(), getHeight());

    return _contentLoaded;
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
    if (_contentLoaded)
    {
        for (auto& item : _cubeTransformsRes)
            delete item;

        delete _ambient;
        delete _dynamicData;
        delete _UAVRes;
        delete _readBack;

        //_pHeap->Release(); // TODO: why Release doesn't work? :(
        _descHeap->Release();
    }

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

    if (DELTATIME > 1.0f)
    {
        distribution.TrySpawnStep();
        DELTATIME = 0.0f;
    }
    DELTATIME += updateEvent.elapsedTime;

    static float angle = 0.0f;
    angle += 0.00035;
    for (int i = 0; i < CUBES_SIZE; ++i)
    {
        for (int j = 0; j < CUBES_SIZE; ++j)
        {
            _transfP[updateEvent.frameIndex][i + j * CUBES_SIZE] = DirectX::XMMatrixRotationY(angle) * DirectX::XMMatrixTranslation(i * 2 - CUBES_SIZE, j * 2 - CUBES_SIZE, 0.0f);
        }
    }
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

    // Execute the Compute shader
    {
        transitionResource(commandList, _UAVRes->_resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE , D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        commandList->SetPipelineState(_pipelineComputeState.Get());
        commandList->SetComputeRootSignature(_rootComputeSignature.Get());

        commandList->SetComputeRootShaderResourceView(0, _dynamicData->OffsetGPU(0));
        //commandList->SetComputeRootUnorderedAccessView(1, _UAVRes->OffsetGPU(0));

        ID3D12DescriptorHeap* heaps = { _descHeap.Get() };
        commandList->SetDescriptorHeaps(1, &heaps);

        D3D12_GPU_DESCRIPTOR_HANDLE offset2 = _descHeap->GetGPUDescriptorHandleForHeapStart();
        offset2.ptr += 32 * 0;
        commandList->SetComputeRootDescriptorTable(1, offset2);

        uint32_t x, y, z;
        x = 64 / 8; // 64 / 2
        y = 64 / 8; // 64 / 2
        z = 1;
        commandList->Dispatch(x, y, z);

        transitionResource(commandList, _UAVRes->_resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

       // commandList->CopyResource(_readBack->_resource.Get(), _UAVRes->_resource.Get());
    }

    // Execute the TriangleRender shader
    {
        commandList->SetPipelineState(pipeline.GetPipelineState().Get());
        commandList->SetGraphicsRootSignature(pipeline.GetRootSignature().Get());

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
        commandList->IASetIndexBuffer(&_indexBufferView);

        commandList->RSSetViewports(1, &_viewport);
        commandList->RSSetScissorRects(1, &_scissorRect);

        commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

        XMMATRIX mvpMatrix = XMMatrixMultiply(camera.View(), camera.Projection());
        commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);
        commandList->SetGraphicsRootConstantBufferView(1, _ambient->OffsetGPU(0));

        // Update the MVP matrix
        commandList->SetGraphicsRootShaderResourceView(2, _cubeTransformsRes[renderEvent.frameIndex]->OffsetGPU(0));

        ID3D12DescriptorHeap* heaps = { _descHeap.Get() };
        commandList->SetDescriptorHeaps(1, &heaps);

        D3D12_GPU_DESCRIPTOR_HANDLE offset2 = _descHeap->GetGPUDescriptorHandleForHeapStart();
        offset2.ptr += 32 * 1;
        commandList->SetGraphicsRootDescriptorTable(3, offset2);

        commandList->DrawIndexedInstanced(static_cast<UINT>(_model.GetIndices().size()), CUBES_SIZE * CUBES_SIZE, 0, 0, 0);
    }

    // Present
    {
        transitionResource(commandList, backBuffer,
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        _fenceValues[currentBackBufferIndex] = commandQueue->executeCommandList(commandList);

        currentBackBufferIndex = _window->present();

        
        commandQueue->waitForFenceValue(_fenceValues[0]);
    }

    //float* result = (float*)_readBack->Map();
    //int dd = 23;
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
        distribution.Reset();
        distribution.Init();
        break;
    }
}

void RenderCubeExample::onMouseScroll(MouseScrollEvent& e)
{
    //_FoV -= e.scrollDelta;
    //_FoV = clamp(_FoV, 12.0f, 90.0f);
    //camera.SetFOV(_FoV);

    //char buffer[256];
    //sprintf_s(buffer, "FoV: %f\n", _FoV);
    //OutputDebugStringA(buffer);
}

void RenderCubeExample::onMouseMoved(MouseMoveEvent& e)
{
    if ((e.relativeX != 0 || e.relativeY != 0) && _isCameraMoving)
        camera.Update(e.relativeX, e.relativeY);
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
