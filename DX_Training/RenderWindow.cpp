#include "pch.h"

#include "RenderWindow.h"
#include "Application.h"
#include "CommandQueue.h"
#include "Interfaces/IGame.h"
#include "Events/ResizeEvent.h"
#include "Events/RenderEvent.h"
#include "Events/UpdateEvent.h"

Window::Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync)
    : _hWnd(hWnd)
    , _windowName(windowName)
    , _width(clientWidth)
    , _height(clientHeight)
    , _vSync(vSync)
    , _fullscreen(false)
    , _frameCounter(0)
{
    Application& app = Application::get();

    _isTearingSupported = app.isTearingSupported();

    _dxgiSwapChain = createSwapChain();
    _d3d12RTVDescriptorHeap = app.createDescriptorHeap(BUFFER_COUNT, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    _RTVDescriptorSize = app.getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    updateRenderTargetViews();
}

Window::~Window()
{
    // Window should be destroyed with Application::DestroyWindow before
    // the window goes out of scope.
    assert(!_hWnd && "Use Application::DestroyWindow before destruction.");
}

HWND Window::getWindowHandle() const
{
    return _hWnd;
}

const std::wstring& Window::getWindowName() const
{
    return _windowName;
}

void Window::show()
{
    ::ShowWindow(_hWnd, SW_SHOW);
}

/**
* Hide the window.
*/
void Window::hide()
{
    ::ShowWindow(_hWnd, SW_HIDE);
}

void Window::destroy()
{
    if (auto pGame = _game.lock())
    {
        // Notify the registered game that the window is being destroyed.
        pGame->onWindowDestroy();
    }
    if (_hWnd)
    {
        DestroyWindow(_hWnd);
        _hWnd = nullptr;
    }
}

int Window::getWidth() const
{
    return _width;
}

int Window::getHeight() const
{
    return _height;
}

bool Window::isVSync() const
{
    return _vSync;
}

void Window::setVSync(bool vSync)
{
    _vSync = vSync;
}

void Window::toggleVSync()
{
    setVSync(!_vSync);
}

bool Window::isFullScreen() const
{
    return _fullscreen;
}

// Set the fullscreen state of the window.
void Window::setFullscreen(bool fullscreen)
{
    if (_fullscreen != fullscreen)
    {
        _fullscreen = fullscreen;

        if (_fullscreen) // Switching to fullscreen.
        {
            // Store the current window dimensions so they can be restored 
            // when switching out of fullscreen state.
            ::GetWindowRect(_hWnd, &_windowRect);

            // Set the window style to a borderless window so the client area fills
            // the entire screen.
            UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

            ::SetWindowLongW(_hWnd, GWL_STYLE, windowStyle);

            // Query the name of the nearest display device for the window.
            // This is required to set the fullscreen dimensions of the window
            // when using a multi-monitor setup.
            HMONITOR hMonitor = ::MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX monitorInfo = {};
            monitorInfo.cbSize = sizeof(MONITORINFOEX);
            ::GetMonitorInfo(hMonitor, &monitorInfo);

            ::SetWindowPos(_hWnd, HWND_TOPMOST,
                monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(_hWnd, SW_MAXIMIZE);
        }
        else
        {
            // Restore all the window decorators.
            ::SetWindowLong(_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

            ::SetWindowPos(_hWnd, HWND_NOTOPMOST,
                _windowRect.left,
                _windowRect.top,
                _windowRect.right - _windowRect.left,
                _windowRect.bottom - _windowRect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(_hWnd, SW_NORMAL);
        }
    }
}

void Window::toggleFullscreen()
{
    setFullscreen(!_fullscreen);
}


void Window::registerCallbacks(std::shared_ptr<IGame> pGame)
{
    _game = pGame;

    return;
}

void Window::onUpdate(UpdateEvent&)
{
    _updateClock.tick();

    if (auto pGame = _game.lock())
    {
        _frameCounter++;

        UpdateEvent updateEventArgs(_updateClock.getDeltaSeconds(), _updateClock.getTotalSeconds());
        pGame->onUpdate(updateEventArgs);
    }
}

void Window::onRender(RenderEvent&)
{
    _renderClock.tick();

    if (auto pGame = _game.lock())
    {
        RenderEvent renderEventArgs(_renderClock.getDeltaSeconds(), _renderClock.getTotalSeconds());
        pGame->onRender(renderEventArgs);
    }
}

void Window::onKeyPressed(KeyEvent& e)
{
    if (auto pGame = _game.lock())
    {
        pGame->onKeyPressed(e);
    }
}

void Window::onKeyReleased(KeyEvent& e)
{
    if (auto pGame = _game.lock())
    {
        pGame->onKeyReleased(e);
    }
}

// The mouse was moved
void Window::onMouseMoved(MouseMoveEvent& e)
{
    if (auto pGame = _game.lock())
    {
        e.relativeX = e.x - _lastMouseMoveEvent.x;
        e.relativeY = e.y - _lastMouseMoveEvent.y;

        pGame->onMouseMoved(e);

        _lastMouseMoveEvent = e;
    }
}

// A button on the mouse was pressed
void Window::onMouseButtonPressed(MouseButtonEvent& e)
{
    if (auto pGame = _game.lock())
    {
        pGame->onMouseButtonPressed(e);
    }
}

// A button on the mouse was released
void Window::onMouseButtonReleased(MouseButtonEvent& e)
{
    if (auto pGame = _game.lock())
    {
        pGame->onMouseButtonReleased(e);
    }
}

// The mouse wheel was moved.
void Window::onMouseScroll(MouseScrollEvent& e)
{
    if (auto pGame = _game.lock())
    {
        pGame->onMouseScroll(e);
    }
}

void Window::onResize(ResizeEvent& e)
{
    // Update the client size.
    if (_width != e.width || _height != e.height)
    {
        _width = std::max(1, e.width);
        _height = std::max(1, e.height);

        Application::get().flush();

        for (int i = 0; i < BUFFER_COUNT; ++i)
        {
            _d3d12BackBuffers[i].Reset();
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        Helper::throwIfFailed(_dxgiSwapChain->GetDesc(&swapChainDesc));
        Helper::throwIfFailed(_dxgiSwapChain->ResizeBuffers(BUFFER_COUNT, _width,
            _height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

        _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

        updateRenderTargetViews();
    }

    if (auto pGame = _game.lock())
    {
        pGame->onResize(e);
    }
}

Microsoft::WRL::ComPtr<IDXGISwapChain4> Window::createSwapChain()
{
    Application& app = Application::get();

    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    Helper::throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = _width;
    swapChainDesc.Height = _height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = BUFFER_COUNT;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = _isTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    ID3D12CommandQueue* pCommandQueue = app.getCommandQueue()->getD3D12CommandQueue().Get();

    ComPtr<IDXGISwapChain1> swapChain1;
    Helper::throwIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        pCommandQueue,
        _hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    Helper::throwIfFailed(dxgiFactory4->MakeWindowAssociation(_hWnd, DXGI_MWA_NO_ALT_ENTER));

    Helper::throwIfFailed(swapChain1.As(&dxgiSwapChain4));

    _currentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

    return dxgiSwapChain4;
}

// Update the render target views for the swapchain back buffers.
void Window::updateRenderTargetViews()
{
    auto device = Application::get().getDevice();

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        Helper::throwIfFailed(_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        _d3d12BackBuffers[i] = backBuffer;

        rtvHandle.Offset(_RTVDescriptorSize);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE Window::getCurrentRenderTargetView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        _currentBackBufferIndex, _RTVDescriptorSize);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Window::getCurrentBackBuffer() const
{
    return _d3d12BackBuffers[_currentBackBufferIndex];
}

UINT Window::getCurrentBackBufferIndex() const
{
    return _currentBackBufferIndex;
}

UINT Window::present()
{
    UINT syncInterval = _vSync ? 1 : 0;
    UINT presentFlags = _isTearingSupported && !_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    Helper::throwIfFailed(_dxgiSwapChain->Present(syncInterval, presentFlags));
    _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

    return _currentBackBufferIndex;
}
