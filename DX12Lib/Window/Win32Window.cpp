#include "stdafx.h"

#include "Win32Window.h"

#include "Events/KeyEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseScrollEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/ResizeEvent.h"

namespace Core
{
    using Events::MouseMoveEvent;
    using Events::MouseScrollEvent;
    using Events::MouseButtonEvent;
    using Events::ResizeEvent;

    Win32Window::Win32Window(HINSTANCE hInstance, int width, int height, const std::wstring& title, bool vSync)
        : _eventListener(nullptr)
        , _vSync(vSync)
        , _title(title)
        , _width(width)
        , _height(height)
        , _fullscreen(false)
    {
        RECT windowRect = { 0, 0, width, height };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        _windowHandle = CreateWindowW(L"DX12WindowClass", title.c_str(),
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr, nullptr, hInstance, this);

        if (!_windowHandle)
        {
            MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
            return;
        }
    }

    void Win32Window::Show()
    {
        ::ShowWindow(_windowHandle, SW_SHOW);
    }

    void Win32Window::Hide()
    {
        ::ShowWindow(_windowHandle, SW_HIDE);
    }

    void Win32Window::AddEventListener(Events::IWindowEventListener* listener)
    {
        _eventListener = listener;
    }

    void Win32Window::RemoveEventListener()
    {
        _eventListener = nullptr;
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
        static Events::IWindowEventListener* listener = nullptr;
        if (!listener)
        {
            //listener = (IWindowEventListener*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
            listener = _eventListener;
        }

        if (listener)
        {
            switch (message)
            {
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
                //bool lButton = (wParam & MK_LBUTTON) != 0;
                //bool rButton = (wParam & MK_RBUTTON) != 0;
                //bool mButton = (wParam & MK_MBUTTON) != 0;
                //bool shift = (wParam & MK_SHIFT) != 0;
                //bool control = (wParam & MK_CONTROL) != 0;

                //int x = ((int)(short)LOWORD(lParam));
                //int y = ((int)(short)HIWORD(lParam));

                //MouseMoveEvent mouseMotionEventArgs(lButton, mButton, rButton, control, shift, x, y);
                //listener->OnMouseMoved(mouseMotionEventArgs);
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
