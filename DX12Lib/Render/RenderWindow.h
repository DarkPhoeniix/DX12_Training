#pragma once

#include "Utility/HighResolutionClock.h"
#include "Events/MouseMoveEvent.h"

class IGame;
class Window;
class UpdateEvent;
class RenderEvent;
class MouseButtonEvent;
class MouseScrollEvent;
class KeyEvent;
class ResizeEvent;

class Window
{
public:
    static constexpr UINT BUFFER_COUNT = 3;

    HWND getWindowHandle() const;

    void destroy();

    const std::wstring& getWindowName() const;

    int getWidth() const;
    int getHeight() const;

    bool isVSync() const;
    void setVSync(bool vSync);
    void toggleVSync();

    bool isFullScreen() const;
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();

    void show();
    void hide();

    UINT getCurrentBackBufferIndex() const;
    UINT present2();
    UINT present();

    D3D12_CPU_DESCRIPTOR_HANDLE getCurrentRenderTargetView() const;
    ComPtr<ID3D12Resource> getCurrentBackBuffer() const;

    ComPtr<IDXGISwapChain4> GetSwapChain();

protected:
    friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    friend class Application;
    friend class IGame;

    Window() = delete;
    Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync);
    virtual ~Window();

    void registerCallbacks(std::shared_ptr<IGame> pGame);

    virtual void onUpdate(UpdateEvent& e);
    virtual void onRender(RenderEvent& e);

    virtual void onKeyPressed(KeyEvent& e);
    virtual void onKeyReleased(KeyEvent& e);

    virtual void onMouseMoved(MouseMoveEvent& e);
    virtual void onMouseButtonPressed(MouseButtonEvent& e);
    virtual void onMouseButtonReleased(MouseButtonEvent& e);
    virtual void onMouseScroll(MouseScrollEvent& e);

    virtual void onResize(ResizeEvent& e);

    ComPtr<IDXGISwapChain4> createSwapChain();

    void updateRenderTargetViews();

public:
    Window(const Window& copy) = delete;
    Window& operator=(const Window& other) = delete;

    HWND _hWnd;

    std::wstring _windowName;

    int _width;
    int _height;
    bool _vSync;
    bool _fullscreen;

    HighResolutionClock _updateClock;
    HighResolutionClock _renderClock;
    uint64_t _frameCounter;

    std::weak_ptr<IGame> _game;

    ComPtr<IDXGISwapChain4> _dxgiSwapChain;
    ComPtr<ID3D12DescriptorHeap> _d3d12RTVDescriptorHeap;
    ComPtr<ID3D12Resource> _d3d12BackBuffers[BUFFER_COUNT];

    UINT _RTVDescriptorSize;
    UINT _currentBackBufferIndex;

    RECT _windowRect;
    bool _isTearingSupported;

    MouseMoveEvent _lastMouseMoveEvent = MouseMoveEvent(false, false, false, false, false, 0, 0);
};
