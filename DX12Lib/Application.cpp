
#include "stdafx.h"

#include "Application.h"

#include "Render/DXRenderer.h"
#include "Window/Win32Window.h"
#include "Events/KeyEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/MouseScrollEvent.h"
#include "Events/RenderEvent.h"
#include "Events/ResizeEvent.h"
#include "Events/UpdateEvent.h"

namespace
{
    constexpr wchar_t WINDOW_CLASS_NAME[] = L"DX12WindowClass";

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

Application& Application::Get()
{
    return *_appInstance;
}

std::shared_ptr<Win32Window> Application::CreateWin32Window(int width, int height, const std::wstring& title, bool vSync)
{
    RECT windowRect = { 0, 0, width, height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindowW(WINDOW_CLASS_NAME, title.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, _hInstance, nullptr);

    if (!hWnd)
    {
        MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    std::shared_ptr<Win32Window> pWindow = std::make_shared<Win32Window>(hWnd, title, width, height, vSync);

    return pWindow;
}

void Application::Init(HINSTANCE hInstance)
{
    _appInstance = new Application(hInstance);
}

int Application::Run(std::shared_ptr<DXRenderer> pApp)
{
    if (!pApp->LoadContent()) return 1;


    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            pApp->OnUpdate();
            pApp->OnRender();
        }
        
    }

    pApp->UnloadContent();

    return static_cast<int>(msg.wParam);
}

void Application::Quit(int exitCode)
{
    PostQuitMessage(exitCode);
}

Application::Application(HINSTANCE hInstance)
    : _hInstance(hInstance)
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugInterface;
    Helper::throwIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif

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
        MessageBoxA(NULL, "Unable to register the window class.", "Error", MB_OK | MB_ICONERROR);
}

Application::~Application()
{
    if (_appInstance)
        delete _appInstance;

    _appInstance = nullptr;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Core::Win32Window* window = (Core::Win32Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

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

    if (window)
    {
        LRESULT result = window->WindowProcCallback(hwnd, message, wParam, lParam);
        if (result != -1)
            return result;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
