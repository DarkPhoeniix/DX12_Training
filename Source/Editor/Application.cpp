#include "EditorPCH.h"

#include "Application.h"

#include "SwapChain.h"

#include "Events/RenderEvent.h"
#include "Events/UpdateEvent.h"
#include "Input/InputDevice.h"

#include "Core/DXRenderer.h"
#include "Helpers/DebugInfo.h"
#include "Window/Win32Window.h"

#include "Resources/resource.h"

#include "RHI/CommandQueue.h"

#include "Renderer/Window/Win32Window.h"

using namespace core;
using namespace render;

namespace
{
    constexpr wchar_t WINDOW_CLASS_NAME[] = L"DX12WindowClass";
}

Application* Application::_instance = nullptr;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    GUI_WndProc(hwnd, message, wParam, lParam);

    switch (message)
    {
    case WM_CREATE:
    {
        // passing pointer for Win32Window into USERDATA section
        // by some unknown reason, it can be missed by default
        CREATESTRUCTA* pCreation = reinterpret_cast<CREATESTRUCTA*>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCreation->lpCreateParams);

    }
    break;
    }

    core::Win32Window* window = (core::Win32Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (window)
    {
        LRESULT result = window->WindowProcCallback(hwnd, message, wParam, lParam);
        if (result != -1)
        {
            return result;
        }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

Application::Application(HINSTANCE hInstance)
    : _hInstance(hInstance)
    , _currentFrame(nullptr)
    , _device(rhi::CreateDevice(rhi::BackendAPI::D3D12))
{
    _frames.resize(rhi::BACK_BUFFER_COUNT, _device.get());
    _fencePool = std::make_unique<FencePool>(_device.get());

    DebugInfo::Init(_device.get());

    _RegisterWindowClass(hInstance);
}

Application::~Application()
{
}

void Application::Init(HINSTANCE hInstance)
{
    logging::Logger::Init("engine.log");

    _instance = new Application(hInstance);
}

int Application::Run(const WindowParams& windowParams, std::string cmdLine)
{
    // Initialization
    {
        _win32Window = CreateWin32Window(windowParams.Width, windowParams.Height, windowParams.Name, windowParams.VSyns);
        _renderer = std::make_unique<render::DXRenderer>(_device.get(), _win32Window->GetWindowHandle());

        _swapChain = _device->CreateSwapChain(_win32Window->GetWindowHandle(), _win32Window->GetWidth(), _win32Window->GetHeight(), _win32Window->IsVSync());
        _win32Window->SetSwapChain(_swapChain.get());
        _device->BindSwapChain(_swapChain.get());

        _allocs.Init(_device.get());
        _fencePool->Init();

        for (int i = 0; i < rhi::BACK_BUFFER_COUNT; ++i)
        {
            int nextIndex = (i + 1) % rhi::BACK_BUFFER_COUNT;
            int prevIndex = (i == 0) ? (rhi::BACK_BUFFER_COUNT - 1) : (i - 1);

            Frame& frame = _frames[i];
            frame.Index = i;
            frame.Next = &_frames[nextIndex];
            frame.Prev = &_frames[prevIndex];

            frame.SetSyncPoint(nullptr);
            frame.SetAllocatorPool(&_allocs);
            frame.SetFencePool(_fencePool.get());

            frame.Init((uint32_t)_win32Window->GetWidth(), (uint32_t)_win32Window->GetHeight());
        }

        _currentFrame = &_frames[0];

        _renderer->SetFrame(*_currentFrame);
    }

    _win32Window->AddEventListener(_renderer.get());

    events::InputDevice::Instance().AddInputObserver(_renderer.get());

    TaskGPU* uploadTask = _currentFrame->CreateTask(rhi::CommandListType::Graphics, nullptr);
    if (!_renderer->LoadContent(uploadTask, cmdLine))
    {
        return 1;
    }
    _currentFrame->SetSyncPoint(uploadTask->GetFence());
    _ExecuteFrameTasks();

    _editor = std::make_shared<gui::Editor>(_device.get(), _win32Window->GetWindowHandle());
    _editor->Init(_renderer->GetCurrentScene());
    _win32Window->AddEventListener(_editor.get());
    _editor->SetRenderGraph(_renderer->GetRenderGraph());
    _editor->AddGUIRenderPass();

    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        events::InputDevice::Instance().PollEvents();

        _editor->NewFrame();

        _UpdateCall();
        _RenderCall();

        _ExecuteFrameTasks();
        _currentFrame = _currentFrame->Next;
    }

    for (Frame& frame : _frames)
    {
        frame.WaitCPU();
    }

    _renderer->UnloadContent();

    return static_cast<int>(msg.wParam);
}

void Application::Quit(int exitCode)
{
    Instance()->_editor->Destroy();
    DebugInfo::Destroy();

    PostQuitMessage(exitCode);

    logging::Logger::Shutdown();

    if (_instance)
    {
        delete _instance;
    }
    _instance = nullptr;
}

Application* Application::Instance()
{
    return _instance;
}

std::unique_ptr<core::Win32Window> Application::CreateWin32Window(int width, int height, const std::wstring& title, bool vSync)
{
    std::unique_ptr<core::Win32Window> pWindow = std::make_unique<core::Win32Window>(Instance()->_hInstance, width, height, title, vSync);

    pWindow->Show();

    return std::move(pWindow);
}

void Application::_RegisterWindowClass(HINSTANCE hInstance)
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEXW wndClass = { 0 };

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = &WindowProc;
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.hIcon = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = WINDOW_CLASS_NAME;
    wndClass.hIconSm = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_ICON1));

    if (!RegisterClassExW(&wndClass))
    {
        MessageBoxA(NULL, "Unable to register the window class.", "Error", MB_OK | MB_ICONERROR);
    }
}

void Application::_UpdateCall()
{
    _updateClock.Tick();

    events::UpdateEvent updateEvent(_updateClock.GetDeltaSeconds(), _updateClock.GetTotalSeconds(), _currentFrame->Index);
    _renderer->OnUpdate(updateEvent);
}

void Application::_RenderCall()
{
    _renderClock.Tick();

    events::RenderEvent renderEvent(_updateClock.GetDeltaSeconds(), _updateClock.GetTotalSeconds(), _currentFrame->Index);
    _renderer->SetFrame(*_currentFrame);
    _renderer->OnRender(renderEvent);
}

void Application::_ExecuteFrameTasks()
{
    for (TaskGPU& task : _currentFrame->GetTasks())
    {
        std::vector<TaskGPU*> dependencies;

        // wait
        for (const std::string& dependency : task.GetDependencies())
        {
            if (TaskGPU* dependentTask = _currentFrame->GetTask(dependency))
            {
                dependencies.push_back(dependentTask);
            }
        }

        rhi::CommandQueue* queue = _device->GetQueue(task.GetType());
        rhi::Fence* fence = task.GetFence();

        for (TaskGPU* d : dependencies)
        {
            rhi::CommandQueue* dQueue = _device->GetQueue(d->GetType());
            rhi::Fence* dFence = d->GetFence();

            dQueue->Wait(dFence, dFence->GetValue());
        }

        std::vector<rhi::CommandList*> frameCommandLists = { task.GetCommandList() };

        queue->ExecuteCommandLists(frameCommandLists);

        if (task.GetName() == "present_pass")
        {
            _device->Present();
            _currentFrame->SetSyncPoint(task.GetFence());
        }
        queue->Signal(fence, fence->GetValue());
    }
}
