#include "stdafx.h"

#include "Application.h"

#include "DXObjects/SwapChain.h"
#include "DXObjects/Device.h"
#include "Events/KeyEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/MouseScrollEvent.h"
#include "Events/RenderEvent.h"
#include "Events/ResizeEvent.h"
#include "Events/UpdateEvent.h"
#include "Render/DXRenderer.h"
#include "Window/Win32Window.h"

using namespace Core;

namespace
{
    constexpr wchar_t WINDOW_CLASS_NAME[] = L"DX12WindowClass";

    static Application* _appInstance;

    // Convert the message ID into a MouseButton ID
    Core::Input::MouseButtonEvent::MouseButton DecodeMouseButton(UINT messageID)
    {
        Core::Input::MouseButtonEvent::MouseButton mouseButton = Core::Input::MouseButtonEvent::None;
        switch (messageID)
        {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        {
            mouseButton = Core::Input::MouseButtonEvent::Left;
        }
        break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        {
            mouseButton = Core::Input::MouseButtonEvent::Right;
        }
        break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        {
            mouseButton = Core::Input::MouseButtonEvent::Middle;
        }
        break;
        }

        return mouseButton;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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

    Core::Win32Window* window = (Core::Win32Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

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
    , _DXDevice(Core::Device::GetDXDevice())
    , _currentFrame(&_frames[0])
{
    _RegisterWindowClass(hInstance);
}

Application::~Application()
{
    if (_appInstance)
    {
        delete _appInstance;
    }

    _appInstance = nullptr;
    _DXDevice = nullptr;
}

void Application::Init(HINSTANCE hInstance)
{
    _appInstance = new Application(hInstance);
}

int Application::Run(std::shared_ptr<DXRenderer> pApp)
{
    _swapChain.Init(_win32Window);

    _allocs.Init(_DXDevice);
    _fencePool.Init(_DXDevice);

    for (int i = 0; i < 3; ++i)
    {
        Frame& frame = _frames[i];
        frame.Index = i;
        frame.Next = &_frames[(i + 1) % 3];
        frame.Prev = &_frames[(i + 3 - 1) % 3];

        frame.SetDXDevice(_DXDevice);
        frame.SetDirectQueue(Core::Device::GetStreamQueue());
        frame.SetComputeQueue(Core::Device::GetComputeQueue());
        frame.SetCopyQueue(Core::Device::GetCopyQueue());

        frame.SetSyncFrame(nullptr);
        frame.SetAllocatorPool(&_allocs);
        frame.SetFencePool(&_fencePool);

        frame.Init(_swapChain);
    }

    if (!pApp->LoadContent(_currentFrame->CreateTask(D3D12_COMMAND_LIST_TYPE_COPY, nullptr)))
    {
        return 1;
    }

    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        _UpdateCall(pApp);
        _RenderCall(pApp);

        _ExecuteFrameTasks();
        _currentFrame = _currentFrame->Next;
    }

    pApp->UnloadContent();

    return static_cast<int>(msg.wParam);
}

void Application::Quit(int exitCode)
{
    PostQuitMessage(exitCode);
}

Application& Application::Get()
{
    if (!_appInstance)
    {
        Logger::Log(LogType::Error, "Application is uninitialized");
    }

    return *_appInstance;
}

std::shared_ptr<Core::Win32Window> Application::CreateWin32Window(int width, int height, const std::wstring& title, bool vSync)
{
    std::shared_ptr<Core::Win32Window> pWindow = std::make_shared<Core::Win32Window>(Get()._hInstance, width, height, title, vSync);
    Get()._win32Window = pWindow;

    pWindow->Show();

    return pWindow;
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
    //wndClass.hIcon = LoadIcon(_hInstance, MAKEINTRESOURCE(APP_ICON));
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = WINDOW_CLASS_NAME;
    //wndClass.hIconSm = LoadIcon(_hInstance, MAKEINTRESOURCE(APP_ICON));

    if (!RegisterClassExW(&wndClass))
    {
        MessageBoxA(NULL, "Unable to register the window class.", "Error", MB_OK | MB_ICONERROR);
    }
}

void Application::_UpdateCall(std::shared_ptr<DXRenderer> pApp)
{
    _updateClock.tick();

    Input::UpdateEvent updateEvent(_updateClock.getDeltaSeconds(), _updateClock.getTotalSeconds(), _currentFrame->Index);
    pApp->OnUpdate(updateEvent);
}

void Application::_RenderCall(std::shared_ptr<DXRenderer> pApp)
{
    _renderClock.tick();

    Input::RenderEvent renderEvent(_updateClock.getDeltaSeconds(), _updateClock.getTotalSeconds(), _currentFrame->Index);
    pApp->OnRender(renderEvent, *_currentFrame);
}

void Application::_ExecuteFrameTasks()
{
    for (TaskGPU& task : _currentFrame->GetTasks())
    {
        std::vector<TaskGPU*> dependencies;

        // wait
        for (const std::string& dependency : task.GetDependencies())
        {
            dependencies.push_back(_currentFrame->GetTask(dependency));
        }

        for (TaskGPU* d : dependencies)
        {
            task.GetCommandQueue()->Wait(d->GetFence()->GetFence().Get(), d->GetFenceValue());
        }

        std::vector<ID3D12CommandList*> frameCommandLists;
        frameCommandLists.reserve(task.GetCommandLists().size());
        for (auto cl : task.GetCommandLists())
        {
            frameCommandLists.push_back(cl.Get());
        }

        task.GetCommandQueue()->ExecuteCommandLists(frameCommandLists.size(), frameCommandLists.data());

        if (task.GetName() == "present")
        {
            _swapChain.Present();
            _currentFrame->SetSyncFrame(task.GetFence());
        }
        task.GetCommandQueue()->Signal(task.GetDXFence(), task.GetFenceValue());
    }
}
