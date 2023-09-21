#pragma once

#include "Interfaces\IRenderWindow.h"

class Game;

class Window
{
public:
    // Number of swapchain back buffers.
    static const UINT BufferCount = 3;

    HWND GetWindowHandle() const;

    void Destroy();

    const std::wstring& GetWindowName() const;

    int GetClientWidth() const;
    int GetClientHeight() const;

    bool IsVSync() const;
    void SetVSync(bool vSync);
    void ToggleVSync();

    bool IsFullScreen() const;

    void SetFullscreen(bool fullscreen);
    void ToggleFullscreen();

    void Show();
    void Hide();

    UINT GetCurrentBackBufferIndex() const;
    UINT Present();

    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;
    Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBuffer() const;

protected:
    friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    friend class Application;
    friend class Game;

    Window() = delete;
    Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync);
    virtual ~Window();

    void RegisterCallbacks(std::shared_ptr<Game> pGame);

    virtual void OnUpdate(UpdateEvent& e);
    virtual void OnRender(RenderEvent& e);

    virtual void OnKeyPressed(KeyEvent& e);
    virtual void OnKeyReleased(KeyEvent& e);

    virtual void OnMouseMoved(MouseMoveEvent& e);
    virtual void OnMouseButtonPressed(MouseButtonEvent& e);
    virtual void OnMouseButtonReleased(MouseButtonEvent& e);
    virtual void OnMouseWheel(MouseWheelEvent& e);

    virtual void OnResize(ResizeEvent& e);

    Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain();

    void UpdateRenderTargetViews();

private:
    Window(const Window& copy) = delete;
    Window& operator=(const Window& other) = delete;

    HWND m_hWnd;

    std::wstring m_WindowName;

    int m_ClientWidth;
    int m_ClientHeight;
    bool m_VSync;
    bool m_Fullscreen;

    uint64_t m_FrameCounter;

    std::weak_ptr<Game> m_pGame;

    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_d3d12RTVDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12BackBuffers[BufferCount];

    UINT m_RTVDescriptorSize;
    UINT m_CurrentBackBufferIndex;

    RECT m_WindowRect;
    bool m_IsTearingSupported;
};
