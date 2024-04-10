#include "stdafx.h"

#include "Win32Window.h"

#include "Events/KeyEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseScrollEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/ResizeEvent.h"

namespace Core
{
    using Input::MouseMoveEvent;
    using Input::MouseScrollEvent;
    using Input::MouseButtonEvent;
    using Input::ResizeEvent;

    Win32Window::Win32Window(HWND windowHandle, int width, int height, const std::wstring& title, bool vSync)
        : _windowHandle(windowHandle)
        , _eventListener(nullptr)
        , _vSync(vSync)
        , _title(title)
        , _width(width)
        , _height(height)
        , _fullscreen(false)
    {   }

    void Win32Window::Show()
    {
        ::ShowWindow(_windowHandle, SW_SHOW);
    }

    void Win32Window::Hide()
    {
        ::ShowWindow(_windowHandle, SW_HIDE);
    }

    void Win32Window::AddEventListener(IWindowEventListener* listener)
    {
        _eventListener = listener;
    }

    void Win32Window::RemoveEventListener()
    {
        _eventListener = nullptr;
    }

    std::shared_ptr<Win32Window> Win32Window::CreateWin32Window(int width, int height, const std::wstring& title, bool vSync)
    {
        RECT windowRect = { 0, 0, width, height };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        HWND handleWindow = CreateWindowW(L"DX12WindowClass", title.c_str(),
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr, nullptr, nullptr, nullptr);

        if (!handleWindow)
        {
            MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        std::shared_ptr<Win32Window> pApp = std::make_shared<Win32Window>(handleWindow, width, height, title, vSync);

        return pApp;
    }

    HWND Win32Window::GetWindowHandle() const
    {
        return _windowHandle;
    }

    const std::wstring& Win32Window::GetTitle() const
    {
        return _title;
    }

    int Win32Window::GetWidth() const
    {
        return _width;
    }

    int Win32Window::GetHeight() const
    {
        return _height;
    }

    bool Win32Window::IsVSync() const
    {
        return _vSync;
    }

    void Win32Window::SetVSync(bool vSync)
    {
        _vSync = vSync;
    }

    void Win32Window::ToggleVSync()
    {
        _vSync = !_vSync;
    }

    bool Win32Window::IsFullScreen() const
    {
        return _fullscreen;
    }

    void Win32Window::SetFullscreen(bool fullscreen)
    {
        _fullscreen = fullscreen;
    }

    void Win32Window::ToggleFullscreen()
    {
        _fullscreen = !_fullscreen;
    }

    LRESULT Win32Window::WindowProcCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        static IWindowEventListener* listener = nullptr;
        if (!listener)
        {
            listener = (IWindowEventListener*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
        }

        if (listener)
        {
            switch (message)
            {
            //case WM_PAINT:
            //{
            //    // Delta time will be filled in by the Window.
            //    UpdateEvent updateEventArgs(0.0f, 0.0f);
            //    listener->OnUpdate(updateEventArgs);
            //    RenderEvent renderEventArgs(0.0f, 0.0f);
            //    // Delta time will be filled in by the Window.
            //    listener->OnRender(renderEventArgs);
            //}
            //break;
            case WM_SIZE:
            {
                int width = ((int)(short)LOWORD(lParam));
                int height = ((int)(short)HIWORD(lParam));

                ResizeEvent resizeEventArgs(width, height);
                listener->OnResize(resizeEventArgs);
            }
            break;
            case WM_MOUSEMOVE:
            {
                bool lButton = (wParam & MK_LBUTTON) != 0;
                bool rButton = (wParam & MK_RBUTTON) != 0;
                bool mButton = (wParam & MK_MBUTTON) != 0;
                bool shift = (wParam & MK_SHIFT) != 0;
                bool control = (wParam & MK_CONTROL) != 0;

                int x = ((int)(short)LOWORD(lParam));
                int y = ((int)(short)HIWORD(lParam));

                MouseMoveEvent mouseMotionEventArgs(lButton, mButton, rButton, control, shift, x, y);
                listener->OnMouseMoved(mouseMotionEventArgs);
            }
            break;
            case WM_DESTROY:
            {
                // If there are no more windows, quit the application.
                PostQuitMessage(0);
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
} // namespace Core
