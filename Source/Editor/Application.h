#pragma once

#include "SwapChain.h"

#include "Render/Frame/Frame.h"
#include "Utility/HighResolutionClock.h"

#include "Editor/Editor.h"

namespace core
{
    class Win32Window;
} // namespace core

namespace render
{
    class DXRenderer;
} // namespace render

struct WindowParams
{
    std::uint32_t Width = 0;
    std::uint32_t Height = 0;
    std::wstring Name;
    bool VSyns = false;
};

class Application
{
public:
    Application(const Application& copy) = delete;
    Application& operator=(const Application& copy) = delete;

    static void Init(HINSTANCE hInstance);
    int Run(const WindowParams& windowParams, std::string cmdLine);
    static void Quit(int exitCode = 0);

    static Application* Instance();

private:
    Application(HINSTANCE hInstance);
    ~Application();

    static std::unique_ptr<core::Win32Window> CreateWin32Window(int width, int height, const std::wstring& title, bool vSync = false);

    void _RegisterWindowClass(HINSTANCE hInstance);

    void _UpdateCall();
    void _RenderCall();
    void _ExecuteFrameTasks();

    friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // The application instance handle that this application was created with.
    HINSTANCE _hInstance;

    std::unique_ptr<core::Win32Window> _win32Window;
    std::unique_ptr<rhi::SwapChain> _swapChain;

    std::vector<std::unique_ptr<Frame>> _frames;
    Frame* _currentFrame;

    std::unique_ptr<rhi::Device> _device;

    std::unique_ptr<FencePool> _fencePool;

    HighResolutionClock _updateClock;
    HighResolutionClock _renderClock;
    uint64_t _frameCounter;

    std::shared_ptr<gui::Editor> _editor;

    std::unique_ptr<render::DXRenderer> _renderer;

    static Application* _instance;
};
