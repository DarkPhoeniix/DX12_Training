#pragma once

#include "Utility/HighResolutionClock.h"

class Game;
class Window;
class UpdateEvent;
class RenderEvent;
class MouseButtonEvent;
class MouseMoveEvent;
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

    int getClientWidth() const;
    int getClientHeight() const;

    bool isVSync() const;
    void setVSync(bool vSync);
    void toggleVSync();

    bool isFullScreen() const;
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();

    void show();
    void hide();

    UINT getCurrentBackBufferIndex() const;
    UINT present();

    D3D12_CPU_DESCRIPTOR_HANDLE getCurrentRenderTargetView() const;
    Microsoft::WRL::ComPtr<ID3D12Resource> getCurrentBackBuffer() const;

protected:
    friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    friend class Application;
    friend class Game;

    Window() = delete;
    Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync);
    virtual ~Window();

    void registerCallbacks(std::shared_ptr<Game> pGame);

    virtual void onUpdate(UpdateEvent& e);
    virtual void onRender(RenderEvent& e);

    virtual void onKeyPressed(KeyEvent& e);
    virtual void onKeyReleased(KeyEvent& e);

    virtual void onMouseMoved(MouseMoveEvent& e);
    virtual void onMouseButtonPressed(MouseButtonEvent& e);
    virtual void onMouseButtonReleased(MouseButtonEvent& e);
    virtual void onMouseScroll(MouseScrollEvent& e);

    virtual void onResize(ResizeEvent& e);

    Microsoft::WRL::ComPtr<IDXGISwapChain4> createSwapChain();

    void updateRenderTargetViews();

private:
    Window(const Window& copy) = delete;
    Window& operator=(const Window& other) = delete;

    HWND _hWnd;

    std::wstring _windowName;

    int _clientWidth;
    int _clientHeight;
    bool _vSync;
    bool _fullscreen;

    HighResolutionClock _updateClock;
    HighResolutionClock _renderClock;
    uint64_t _frameCounter;

    std::weak_ptr<Game> _game;

    Microsoft::WRL::ComPtr<IDXGISwapChain4> _dxgiSwapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _d3d12RTVDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> _d3d12BackBuffers[BUFFER_COUNT];

    UINT _RTVDescriptorSize;
    UINT _currentBackBufferIndex;

    RECT _windowRect;
    bool _isTearingSupported;
};
