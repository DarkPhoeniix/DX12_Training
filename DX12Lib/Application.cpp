
#include "stdafx.h"

#include "Application.h"

#include "Interfaces/IGame.h"
#include "Render/RenderWindow.h"
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

    using WindowPtr = std::shared_ptr<Core::Win32Window>;
    using WindowMap = std::map<HWND, WindowPtr>;
    using WindowNameMap = std::map<std::wstring, WindowPtr>;

    static WindowMap g_windows;
    static WindowNameMap g_windowsByName;

    static Application* g_ApplicationInstance = nullptr;

    // A wrapper struct to allow shared pointers for the window class.
    struct MakeWindow : public Core::Win32Window
    {
        MakeWindow(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync)
            : Core::Win32Window(hWnd, clientWidth, clientHeight, windowName, vSync)
        {   }
    };

    static void RemoveWindow(HWND hWnd)
    {
        WindowMap::iterator windowIter = g_windows.find(hWnd);
        if (windowIter != g_windows.end())
        {
            WindowPtr pWindow = windowIter->second;
            g_windowsByName.erase(pWindow->GetTitle());
            g_windows.erase(windowIter);
        }
    }

    // Convert the message ID into a MouseButton ID
    MouseButtonEvent::MouseButton DecodeMouseButton(UINT messageID)
    {
        MouseButtonEvent::MouseButton mouseButton = MouseButtonEvent::None;
        switch (messageID)
        {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        {
            mouseButton = MouseButtonEvent::Left;
        }
        break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        {
            mouseButton = MouseButtonEvent::Right;
        }
        break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        {
            mouseButton = MouseButtonEvent::Middle;
        }
        break;
        }

        return mouseButton;
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WindowPtr pWindow;
    {
        WindowMap::iterator iter = g_windows.find(hwnd);
        if (iter != g_windows.end())
        {
            pWindow = iter->second;
        }
    }

    if (pWindow)
    {
        switch (message)
        {
        case WM_PAINT:
        {
            // Delta time will be filled in by the Window.
            UpdateEvent updateEventArgs(0.0f, 0.0f);
            pWindow->onUpdate(updateEventArgs);
            RenderEvent renderEventArgs(0.0f, 0.0f);
            // Delta time will be filled in by the Window.
            pWindow->onRender(renderEventArgs);
        }
        break;
        case WM_SIZE:
        {
            int width = ((int)(short)LOWORD(lParam));
            int height = ((int)(short)HIWORD(lParam));

            ResizeEvent resizeEventArgs(width, height);
            pWindow->onResize(resizeEventArgs);
        }
        break;
        case WM_DESTROY:
        {
            // If a window is being destroyed, remove it from the 
            // window maps.
            RemoveWindow(hwnd);

            if (g_windows.empty())
            {
                // If there are no more windows, quit the application.
                PostQuitMessage(0);
            }
        }
        break;
        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }
    else
    {
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return 0;
}

std::shared_ptr<Window> Application::CreateWin32Window(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync)
{
    WindowNameMap::iterator windowIter = g_windowsByName.find(windowName);
    if (windowIter != g_windowsByName.end())
    {
        return windowIter->second;
    }

    RECT windowRect = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindowW(WINDOW_CLASS_NAME, windowName.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, _hInstance, nullptr);

    if (!hWnd)
    {
        MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    WindowPtr pWindow = std::make_shared<MakeWindow>(hWnd, windowName, clientWidth, clientHeight, vSync); // TODO: wtf?

    g_windows.insert(WindowMap::value_type(hWnd, pWindow));
    g_windowsByName.insert(WindowNameMap::value_type(windowName, pWindow));

    return pWindow;
}

void Application::DestroyWindow(std::shared_ptr<Window> window)
{
    if (window) window->destroy();
}

void Application::DestroyWindow(const std::wstring& windowName)
{
    WindowPtr pWindow = getWindowByName(windowName);
    if (pWindow)
    {
        DestroyWindow(pWindow);
    }
}

std::shared_ptr<Window> Application::getWindowByName(const std::wstring& windowName)
{
    std::shared_ptr<Window> window;
    WindowNameMap::iterator iter = g_windowsByName.find(windowName);
    if (iter != g_windowsByName.end())
    {
        window = iter->second;
    }

    return window;
}

void Application::Init(HINSTANCE hInstance)
{
    if (!g_ApplicationInstance)
    {
        g_ApplicationInstance = new Application(hInstance);
    }
}

void Application::Destroy()
{
    delete g_ApplicationInstance;
}

Application& Application::get()
{
    assert(g_ApplicationInstance);
    return *g_ApplicationInstance;
}

int Application::run(std::shared_ptr<IGame> pGame)
{
    if (!pGame->initialize()) return 1;
    if (!pGame->LoadContent()) return 2;

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
            // RENDER
        }
        
    }

    // Flush any commands in the commands queues before quiting.
    flush();

    pGame->UnloadContent();
    pGame->destroy();

    return static_cast<int>(msg.wParam);
}

void Application::quit(int exitCode)
{
    PostQuitMessage(exitCode);
}

void Application::flush()
{
}

Application::Application(HINSTANCE)
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
    wndClass.lpfnWndProc = &WndProc;
    wndClass.hInstance = _hInstance;
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //wndClass.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(APP_ICON));
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = WINDOW_CLASS_NAME;
    //wndClass.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(APP_ICON));

    if (!RegisterClassExW(&wndClass))
    {
        MessageBoxA(NULL, "Unable to register the window class.", "Error", MB_OK | MB_ICONERROR);
    }
}

Application::~Application()
{
    flush();
}
