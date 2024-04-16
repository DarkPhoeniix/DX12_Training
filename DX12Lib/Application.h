#pragma once

#include "DXObjects/SwapChain.h"
#include "Render/AllocatorPool.h"
#include "Render/FencePool.h"
#include "Render/Frame.h"

class Win32Window;
class DXRenderer;

class Application
{
public:
    Application(const Application& copy) = delete;
    Application& operator=(const Application& copy) = delete;

    static void Init(HINSTANCE hInstance);
    int Run(std::shared_ptr<DXRenderer> pApp);
    static void Quit(int exitCode = 0);

    static Application* Instance();

    static std::shared_ptr<Core::Win32Window> CreateWin32Window(int width, int height, const std::wstring& title, bool vSync = false);

private:
    Application(HINSTANCE hInstance);
    ~Application();

    void _RegisterWindowClass(HINSTANCE hInstance);

    void _UpdateCall(std::shared_ptr<DXRenderer> pApp);
    void _RenderCall(std::shared_ptr<DXRenderer> pApp);
    void _ExecuteFrameTasks();

    friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // The application instance handle that this application was created with.
    HINSTANCE _hInstance;

    ComPtr<ID3D12Device2> _DXDevice;
    std::shared_ptr<Core::Win32Window> _win32Window;
    SwapChain _swapChain;

    Frame _frames[3];
    Frame* _currentFrame;

    AllocatorPool _allocs;
    FencePool _fencePool;

    HighResolutionClock _updateClock;
    HighResolutionClock _renderClock;
    uint64_t _frameCounter;

    static Application* _instance;
};
