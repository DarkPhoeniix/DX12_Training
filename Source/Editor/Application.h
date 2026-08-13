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

// Application start-up configuration parameters
struct ApplicationConfig
{
    std::uint32_t WindowWidth   = 0;
    std::uint32_t WindowHeight  = 0;
    std::wstring Name           = L""; // TODO: currently not used
    std::wstring ScenePath      = L"";
    bool VSync                  = false;
};

// Entry point of the application, responsible for initializing the application, creating the main window,
// managing the main loop, and handling application-wide resources and events
class Application
{
public:
    Application(const Application& copy) = delete;
    Application& operator=(const Application& copy) = delete;

    static void Init(HINSTANCE hInstance);
    int Run(const ApplicationConfig& config);
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
    int _RunClearOnly();

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
