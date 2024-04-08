
#include "stdafx.h"

#include "Renderer.h"

#include "TextureLoaderDDS.h"
#include "Events/MouseScrollEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/RenderEvent.h"
#include "Events/ResizeEvent.h"
#include "Events/UpdateEvent.h"
#include "Events/KeyEvent.h"
#include "Application.h"
#include "TaskGPU.h"
#include "Frame.h"
#include "Blob.h"

#include <random>
#include <cmath>

using namespace DirectX;

extern Renderer* pShared = nullptr;

namespace
{
    struct Ambient
    {
        XMFLOAT4 Up;
        XMFLOAT4 Down;
    };

    // Clamp a value between a min and max range.
    template<typename T>
    constexpr const T& clamp(const T& val, const T& min, const T& max)
    {
        return (val < min) ? min : (val > max) ? max : val;
    }
}

Renderer::Renderer(const std::wstring& name, int width, int height, bool vSync)
    : super(name, width, height, vSync)
    , _scissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , _viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
    , _contentLoaded(false)
    , _cubeTransformsRes{}
{
    pShared = this;

    for (int i = 0; i < 3; ++i)
    {
        Frame& frame = _frames[i];
        frame.Index = i;
        frame.Next = &_frames[(i + 1) % 3];
        frame.Prev = &_frames[(i + 3 - 1) % 3];

        frame.SetDirectQueue(queueStream);
        frame.SetComputeQueue(queueCompute);
        frame.SetCopyQueue(queueCopy);

        frame.SetSyncFrame(nullptr);
        frame.SetAllocatorPool(&_allocs);
        frame.SetFencePool(&_fencePool);
    }
}

Renderer::~Renderer()
{
}

void Renderer::updateBufferResource(
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

bool Renderer::LoadContent()
{
    auto device = Application::get().getDevice();

    this->_current = &_frames[_window->getCurrentBackBufferIndex()];

    _allocs.Init(device);
    _fencePool.Init(device);

    for (int i = 0; i < 3; ++i)
    {
        Frame& frame = _frames[i];

        frame.SetDirectQueue(queueStream);
        frame.SetComputeQueue(queueCompute);
        frame.SetCopyQueue(queueCopy);

        frame.Init(device, _window->GetSwapChain());
    }

    // Camera Setup
    {
        XMVECTOR pos = XMVectorSet(0.0f, 0.0f, -50.0f, 1.0f);
        XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        float aspectRatio = getWidth() / static_cast<float>(getHeight());

        _camera.LookAt(pos, target, up);
        _camera.SetLens(45.0f, aspectRatio, 0.1f, 1000.0f);
    }

    // ID3D12Heap Setup
    {
        D3D12_HEAP_PROPERTIES heapProperties = {};
        {
            memset(&heapProperties, 0, sizeof(D3D12_HEAP_PROPERTIES));

            heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProperties.CreationNodeMask = 1;
            heapProperties.VisibleNodeMask = 1;

            heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        }
        D3D12_HEAP_DESC heapDesc;
        heapDesc.Alignment = 0;
        heapDesc.Flags = D3D12_HEAP_FLAG_NONE;
        heapDesc.Properties = heapProperties;
        heapDesc.SizeInBytes = 15 * _1MB;
        device->CreateHeap(&heapDesc, IID_PPV_ARGS(&_pHeap));
    }

    EResourceType SRVType = EResourceType::Dynamic | EResourceType::Buffer;
    EResourceType CBVType = EResourceType::Dynamic | EResourceType::Buffer | EResourceType::StrideAlignment;

    ResourceDescription desc;
    desc.SetResourceType(CBVType);
    desc.SetSize({ sizeof(Ambient), 1 });
    desc.SetStride(1);
    desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
    _ambient = new Resource(device, desc);
    _ambient->CreateCommitedResource();
    _ambient->SetName("_ambient");

    Ambient* val = (Ambient*)_ambient->Map();
    val->Up = { 0.0f, 0.8f, 0.7f, 1.0f };
    val->Down = { 0.3f, 0.0f, 0.3f, 1.0f };

    // Descriptor heap
    {
        D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
        descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        descHeapDesc.NodeMask = 0;
        descHeapDesc.NumDescriptors = 32;
        descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_descHeap));
    }

    {
        desc.SetResourceType(EResourceType::Dynamic | EResourceType::Buffer);
        desc.SetSize({ 2, 1 });
        desc.SetStride(sizeof(XMFLOAT2));
        _dynamicData = new Resource(device, desc);
        _dynamicData->CreateCommitedResource();
        _dynamicData->SetName("_dynamicData");

        XMFLOAT4* flData = (XMFLOAT4*)_dynamicData->Map();
        flData[0] = { 0.7f, 0.1f, 0.43f, 1.0f };
        flData[1] = { 0.7f, 0.7f, 1.0f, 1.0f };
    }

    {
        desc.SetResourceType(EResourceType::Unordered | EResourceType::Texture);
        desc.SetSize({ 1024, 1024 });
        desc.SetStride(sizeof(float) * 4);

        _UAVRes = new Resource(device, desc);
        _UAVRes->CreateCommitedResource(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        _UAVRes->SetName("UAV texture");
    }

    TextureLoader::_LoadDDS("dog_tex1.dds", _tex, _blob);

    UINT SRVIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    {
        DescriptorHeapDescription desc;
        desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        desc.SetNumDescriptors(32);
        desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        desc.SetNodeMask(0);

        _texturesDescHeap = std::make_shared<DescriptorHeap>(desc);
        _texturesDescHeap->SetDevice(device);
        _texturesDescHeap->Create();
    }

    {
        HeapDescription desc;
        desc.SetAlignment(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
        desc.SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY_UNKNOWN);
        desc.SetCreationNodeMask(1);
        desc.SetVisibleNodeMask(1);
        desc.SetHeapFlags(D3D12_HEAP_FLAG_NONE);
        desc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
        desc.SetMemoryPoolPreference(D3D12_MEMORY_POOL_UNKNOWN);
        desc.SetSize(_32MB);
        
        _texturesHeap = std::make_shared<Heap>(desc);
        _texturesHeap->SetDevice(device);
        _texturesHeap->Create();
    }

    //_texturesHeap->PlaceResource(*_tex);

    //// UAV setup
    //{
    //    D3D12_CPU_DESCRIPTOR_HANDLE offset = _descHeap->GetCPUDescriptorHandleForHeapStart();
    //    offset.ptr += SRVIncrementSize * 0;

    //    D3D12_UNORDERED_ACCESS_VIEW_DESC _resDesc = {};
    //    _resDesc.Format = _UAVRes->GetResourceDescription().GetFormat();
    //    _resDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    //    _resDesc.Texture2D.MipSlice = 0;
    //    device->CreateUnorderedAccessView(_UAVRes->GetResource().Get(), nullptr, &_resDesc, offset);
    //}

    // SRV setup
    {
        D3D12_CPU_DESCRIPTOR_HANDLE offset2 = _texturesDescHeap->GetHeapStartCPUHandle();
        offset2.ptr += SRVIncrementSize * 1;

        D3D12_SHADER_RESOURCE_VIEW_DESC _resDesc2 = {};
        _resDesc2.Format = _tex->GetResourceDescription().GetFormat();
        _resDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        _resDesc2.Texture2D.MipLevels = 1;
        _resDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        device->CreateShaderResourceView(_tex->GetResource().Get(), &_resDesc2, offset2);
    }

    TaskGPU* task = _current->CreateTask(D3D12_COMMAND_LIST_TYPE_COPY, nullptr);
    task->SetName("Upload Data");

    auto commandList = task->GetCommandLists().front();

    _scene.LoadScene("bowl.fbx", commandList);

    commandList->Close();

    // Create the descriptor heap for the depth-stencil view
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        Helper::throwIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_DSVHeap)));
    }

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    _pipeline.Parse(device.Get(), "Resources\\TriangleRenderPipeline.tech");
    _AABBpipeline.Parse(device.Get(), "Resources\\AABBRenderPipeline.tech");

    // Compute pipeline setup
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

    std::vector<ID3D12CommandList*> comLists;
    comLists.reserve(task->GetCommandLists().size());
    for (auto cl : task->GetCommandLists())
    {
        comLists.push_back(cl.Get());
    }

    task->GetCommandQueue()->ExecuteCommandLists(comLists.size(), comLists.data());
    task->GetCommandQueue()->Signal(task->GetFence()->GetFence().Get(), task->GetFenceValue());

    _contentLoaded = true;

    // Resize/Create the depth buffer.
    resizeDepthBuffer(getWidth(), getHeight());

    return _contentLoaded;
}

void Renderer::resizeDepthBuffer(int width, int height)
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

    this->_current = &_frames[_window->getCurrentBackBufferIndex()];
}

void Renderer::onResize(ResizeEvent& e)
{
    if (e.width != super::getWidth() || e.height != getHeight())
    {
        super::onResize(e);

        _viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
            static_cast<float>(e.width), static_cast<float>(e.height));

        resizeDepthBuffer(e.width, e.height);
    }
}

void Renderer::UnloadContent()
{
    if (_contentLoaded)
    {
        for (auto& item : _cubeTransformsRes)
            delete item;

        delete _ambient;
        _ambient = nullptr;

        delete _dynamicData;
        _dynamicData = nullptr;

        delete _UAVRes;
        _UAVRes = nullptr;

        delete _tex;
        _tex = nullptr;

        _descHeap->Release();
    }

    _contentLoaded = false;
}

void Renderer::onUpdate(UpdateEvent& updateEvent)
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
}

// Transition a resource
void Renderer::transitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
    ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

void Renderer::clearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void Renderer::clearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void Renderer::onRender(RenderEvent& renderEvent)
{
    super::onRender(renderEvent);

    // WAIT CPU

    this->_current->WaitCPU(); // actually ewait GPU
    this->_current->ResetGPU();

    //lock CPU

    // Clear the render targets
    {
        TaskGPU* task = this->_current->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("clean");

        ComPtr<ID3D12GraphicsCommandList2> commandList = task->GetCommandLists().front();

        transitionResource(commandList, _current->_targetTexture.GetResource(),
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        clearRTV(commandList, _current->_targetHeap->GetCPUDescriptorHandleForHeapStart(), clearColor);
        clearDepth(commandList, _current->_depthHeap->GetCPUDescriptorHandleForHeapStart());

        transitionResource(commandList, _UAVRes->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        commandList->Close();
    }

    //// Execute the Compute shader
    //{
    //    TaskGPU* task = this->_current->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, _pipelineComputeState);
    //    task->SetName("compute");

    //    ComPtr<ID3D12GraphicsCommandList2> commandList = task->GetCommandLists().front();

    //    commandList->SetComputeRootSignature(_rootComputeSignature.Get());
    //    commandList->SetComputeRootShaderResourceView(0, _dynamicData->OffsetGPU(0));

    //    ID3D12DescriptorHeap* heaps = { _descHeap.Get() };
    //    commandList->SetDescriptorHeaps(1, &heaps);

    //    D3D12_GPU_DESCRIPTOR_HANDLE offset2 = _descHeap->GetGPUDescriptorHandleForHeapStart();
    //    offset2.ptr += 32 * 0;
    //    commandList->SetComputeRootDescriptorTable(1, offset2);

    //    uint32_t x, y, z;
    //    x = 1024 / 8;//64 / 8;
    //    y = 1024 / 8;//64 / 8;
    //    z = 1;
    //    commandList->Dispatch(x, y, z);

    //    commandList->Close();
    //}

    // Execute the TriangleRender shader
    {
        TaskGPU* task = this->_current->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, _pipeline.GetPipelineState());
        task->SetName("render");

        ComPtr<ID3D12GraphicsCommandList2> commandList = task->GetCommandLists().front();

        static bool ok = true;
        if (ok)
        {
            ok = false;

            CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
            CD3DX12_RESOURCE_DESC buffer = CD3DX12_RESOURCE_DESC::Buffer(1024 * 1024 * 10, D3D12_RESOURCE_FLAG_NONE);

            Helper::throwIfFailed(Application::get().getDevice()->CreateCommittedResource(
                &heapTypeUpload,
                D3D12_HEAP_FLAG_NONE,
                &buffer,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&_intermediateTex)));

            D3D12_SUBRESOURCE_DATA subresources = {};
            subresources.pData = _blob.get();
            subresources.RowPitch = 1024 * 4;
            subresources.SlicePitch = subresources.RowPitch * 1024;

            UpdateSubresources(commandList.Get(), _tex->GetResource().Get(), _intermediateTex.Get(), 0, 0, 1, &subresources);

            transitionResource(commandList, _tex->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }

        task->AddDependency("clean");
        //task->AddDependency("compute");

        transitionResource(commandList, _UAVRes->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        commandList->SetPipelineState(_pipeline.GetPipelineState().Get());
        commandList->SetGraphicsRootSignature(_pipeline.GetRootSignature().Get());

        commandList->RSSetViewports(1, &_viewport);
        commandList->RSSetScissorRects(1, &_scissorRect);

        auto rtv = _current->_targetHeap->GetCPUDescriptorHandleForHeapStart();
        auto dsv = _current->_depthHeap->GetCPUDescriptorHandleForHeapStart();
        commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

        XMMATRIX viewProjMatrix = XMMatrixMultiply(_camera.View(), _camera.Projection());
        commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &viewProjMatrix, 0);
        commandList->SetGraphicsRootConstantBufferView(1, _ambient->OffsetGPU(0));

        ID3D12DescriptorHeap* heaps = { _texturesDescHeap->GetDXDescriptorHeap().Get() };
        commandList->SetDescriptorHeaps(1, &heaps);

        //D3D12_GPU_DESCRIPTOR_HANDLE offset2 = _texturesDescHeap->GetHeapStartGPUHandle();
        //offset2.ptr += 32 * 1;
        //commandList->SetGraphicsRootDescriptorTable(3, offset2);

        //_scene.UploadTextures(commandList, *_texturesHeap, *_texturesDescHeap);

        _scene.Draw(commandList, _camera.GetViewFrustum());


        commandList->SetPipelineState(_AABBpipeline.GetPipelineState().Get());
        commandList->SetGraphicsRootSignature(_AABBpipeline.GetRootSignature().Get());

        commandList->SetGraphicsRoot32BitConstants(1, sizeof(XMMATRIX) / 4, &viewProjMatrix, 0);

        _scene.DrawAABB(commandList);

        commandList->Close();
    }

    // Present
    {
        TaskGPU* task = this->_current->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("present");

        ComPtr<ID3D12GraphicsCommandList2> commandList = task->GetCommandLists().front();

        task->AddDependency("render");

        transitionResource(commandList, _current->_swapChainTexture.GetResource(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);

        transitionResource(commandList, _current->_targetTexture.GetResource(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

        commandList->CopyResource(_current->_swapChainTexture.GetResource().Get(), _current->_targetTexture.GetResource().Get());

        transitionResource(commandList, _current->_swapChainTexture.GetResource(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);

        commandList->Close();
    }

    for (TaskGPU& task : _current->GetTasks())
    {
        std::vector<TaskGPU*> dependencies;

        // wait
        for (const std::string& dependency : task.GetDependencies())
            dependencies.push_back(_current->GetTask(dependency));

        for (TaskGPU* d : dependencies)
            task.GetCommandQueue()->Wait(d->GetFence()->GetFence().Get(), d->GetFenceValue());

        std::vector<ID3D12CommandList*> comLists;
        comLists.reserve(task.GetCommandLists().size());
        for (auto cl : task.GetCommandLists())
        {
            comLists.push_back(cl.Get());
        }

        task.GetCommandQueue()->ExecuteCommandLists(comLists.size(), comLists.data());

        if (task.GetName() == "present")
        {
            _window->present2();
            _current->SetSyncFrame(task.GetFence());
        }
        task.GetCommandQueue()->Signal(task.GetFence()->GetFence().Get(), task.GetFenceValue());
    }

    // unlock CPU

    this->_current = _current->Next;
}

void Renderer::onKeyPressed(KeyEvent& e)
{
    super::onKeyPressed(e);

    XMVECTOR dir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    if (e.key == KeyCode::W)
    {
        dir += _camera.Look();
    }
    if (e.key == KeyCode::S)
    {
        dir -= _camera.Look();
    }
    if (e.key == KeyCode::D)
    {
        dir += _camera.Right();
    }
    if (e.key == KeyCode::A)
    {
        dir -= _camera.Right();
    }
    _camera.Update(dir);

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
        break;
    }
}

void Renderer::onMouseScroll(MouseScrollEvent& e)
{
    //_FoV -= e.scrollDelta;
    //_FoV = clamp(_FoV, 12.0f, 90.0f);
    //camera.SetFOV(_FoV);

    //char buffer[256];
    //sprintf_s(buffer, "FoV: %f\n", _FoV);
    //OutputDebugStringA(buffer);
}

void Renderer::onMouseMoved(MouseMoveEvent& e)
{
    if ((e.relativeX != 0 || e.relativeY != 0) && _isCameraMoving)
        _camera.Update(e.relativeX, e.relativeY);
}

void Renderer::onMouseButtonPressed(MouseButtonEvent& e)
{
    if (e.button == MouseButtonEvent::MouseButton::Right)
        _isCameraMoving = true;
}

void Renderer::onMouseButtonReleased(MouseButtonEvent& e)
{
    if (e.button == MouseButtonEvent::MouseButton::Right)
        _isCameraMoving = false;
}
