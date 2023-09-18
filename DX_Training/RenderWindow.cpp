#include "stdafx.h"

#include "RenderWindow.h"

namespace
{
    ComPtr<IDXGIAdapter4> getAdapter(bool useWarp)
    {
        ComPtr<IDXGIFactory4> dxgiFactory;
        UINT createFactoryFlags = 0;
#if defined(_DEBUG)
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

        throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

        ComPtr<IDXGIAdapter1> dxgiAdapter1;
        ComPtr<IDXGIAdapter4> dxgiAdapter4;

        if (useWarp)
        {
            throwIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
            throwIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
        }
        else
        {
            SIZE_T maxDedicatedVideoMemory = 0;
            for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
                dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

                // Check to see if the adapter can create a D3D12 device without actually 
                // creating it. The adapter with the largest dedicated video memory
                // is favored.
                if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                    SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                        D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
                    dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
                {
                    maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                    throwIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
                }
            }
        }

        return dxgiAdapter4;
    }

    ComPtr<ID3D12Device2> createDevice(ComPtr<IDXGIAdapter4> adapter)
    {
        ComPtr<ID3D12Device2> d3d12Device2;
        throwIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));

        // Enable debug messages in debug mode.
#if defined(_DEBUG)
        ComPtr<ID3D12InfoQueue> pInfoQueue;
        if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
        {
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            // Suppress whole categories of messages
            //D3D12_MESSAGE_CATEGORY Categories[] = {};

            // Suppress messages based on their severity level
            D3D12_MESSAGE_SEVERITY Severities[] =
            {
                D3D12_MESSAGE_SEVERITY_INFO
            };

            // Suppress individual messages by their ID
            D3D12_MESSAGE_ID DenyIds[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
            };

            D3D12_INFO_QUEUE_FILTER NewFilter = {};
            //NewFilter.DenyList.NumCategories = _countof(Categories);
            //NewFilter.DenyList.pCategoryList = Categories;
            NewFilter.DenyList.NumSeverities = _countof(Severities);
            NewFilter.DenyList.pSeverityList = Severities;
            NewFilter.DenyList.NumIDs = _countof(DenyIds);
            NewFilter.DenyList.pIDList = DenyIds;

            throwIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
        }
#endif

        return d3d12Device2;
    }

    ComPtr<ID3D12CommandQueue> createCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = type;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        throwIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

        return d3d12CommandQueue;
    }

    bool checkTearingSupport()
    {
        BOOL allowTearing = FALSE;

        // Rather than create the DXGI 1.5 factory interface directly, we create the
        // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
        // graphics debugging tools which will not support the 1.5 factory interface 
        // until a future update.
        ComPtr<IDXGIFactory4> factory4;
        if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
        {
            ComPtr<IDXGIFactory5> factory5;
            if (SUCCEEDED(factory4.As(&factory5)))
            {
                if (FAILED(factory5->CheckFeatureSupport(
                    DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                    &allowTearing, sizeof(allowTearing))))
                {
                    allowTearing = FALSE;
                }
            }
        }

        return allowTearing == TRUE;
    }

    ComPtr<IDXGISwapChain4> createSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
    {
        ComPtr<IDXGISwapChain4> dxgiSwapChain4;
        ComPtr<IDXGIFactory4> dxgiFactory4;
        UINT createFactoryFlags = 0;
#if defined(_DEBUG)
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

        throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc = { 1, 0 };
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = bufferCount;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        // It is recommended to always allow tearing if tearing support is available.
        swapChainDesc.Flags = checkTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        ComPtr<IDXGISwapChain1> swapChain1;
        throwIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
            commandQueue.Get(),
            hWnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain1));

        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
        // will be handled manually.
        throwIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

        throwIfFailed(swapChain1.As(&dxgiSwapChain4));

        return dxgiSwapChain4;
    }

    ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
    {
        ComPtr<ID3D12DescriptorHeap> descriptorHeap;

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = numDescriptors;
        desc.Type = type;

        throwIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

        return descriptorHeap;
    }

    ComPtr<ID3D12CommandAllocator> createCommandAllocator(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        throwIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

        return commandAllocator;
    }

    ComPtr<ID3D12GraphicsCommandList> createCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12GraphicsCommandList> commandList;
        throwIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

        throwIfFailed(commandList->Close());

        return commandList;
    }

    ComPtr<ID3D12Fence> createFence(ComPtr<ID3D12Device2> device)
    {
        ComPtr<ID3D12Fence> fence;

        throwIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

        return fence;
    }

    HANDLE createEventHandle()
    {
        HANDLE fenceEvent;

        fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        assert(fenceEvent && "Failed to create fence event.");

        return fenceEvent;
    }
}

RenderWindow::RenderWindow(std::uint32_t width, std::uint32_t height, const std::wstring& name)
    : IRenderWindow(width, height, name)
{
}

void RenderWindow::onInit()
{
    loadPipeline();
}

void RenderWindow::onUpdate()
{
}

void RenderWindow::onRender()
{
    // Record all the commands we need to render the scene into the command list.
    populateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { _commandList.Get() };
    _commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    throwIfFailed(_swapChain->Present(1, 0));

    waitForPreviousFrame();
}

void RenderWindow::onDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    waitForPreviousFrame();

    CloseHandle(_fenceEvent);
}

void RenderWindow::loadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIAdapter4> adapter = getAdapter(_useWarpDevice);
    _device = createDevice(adapter);
    _commandQueue = createCommandQueue(_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    _swapChain = createSwapChain(Win32Application::getHwnd(), _commandQueue, _width, _height, g_numFrames);

    _currentBackBufferIndex = _swapChain->GetCurrentBackBufferIndex();

    _RTVDescriptorHeap = createDescriptorHeap(_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, g_numFrames);
    _RTVDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    updateRenderTargetViews();

    for (int i = 0; i < g_numFrames; ++i)
    {
        _commandAllocators[i] = createCommandAllocator(_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    }
    _commandList = createCommandList(_device,
        _commandAllocators[_currentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

    _fence = createFence(_device);
    _fenceEvent = createEventHandle();
}

void RenderWindow::populateCommandList()
{
    auto commandAllocator = _commandAllocators[_currentBackBufferIndex];
    auto backBuffer = _backBuffers[_currentBackBufferIndex];

    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    throwIfFailed(commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    throwIfFailed(_commandList->Reset(commandAllocator.Get(), nullptr));

    // Indicate that the back buffer will be used as a render target.
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    _commandList->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), _currentBackBufferIndex, _RTVDescriptorSize);

    // Record commands.
    const float clearColor[] = { 0.8f, 0.3f, 0.6f, 1.0f };
    _commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Indicate that the back buffer will now be used to present.
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    _commandList->ResourceBarrier(1, &barrier);

    throwIfFailed(_commandList->Close());
}

void RenderWindow::waitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = _fenceValue;
    throwIfFailed(_commandQueue->Signal(_fence.Get(), fence));
    _fenceValue++;

    // Wait until the previous frame is finished.
    if (_fence->GetCompletedValue() < fence)
    {
        throwIfFailed(_fence->SetEventOnCompletion(fence, _fenceEvent));
        WaitForSingleObject(_fenceEvent, INFINITE);
    }

    _currentBackBufferIndex = _swapChain->GetCurrentBackBufferIndex();
}

inline void RenderWindow::updateRenderTargetViews()
{
    auto rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < g_numFrames; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        throwIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        _device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        _backBuffers[i] = backBuffer;

        rtvHandle.Offset(rtvDescriptorSize);
    }
}
