#include "RendererPCH.h"

#include "Win32Window.h"

#include "events/MouseButtonEvent.h"
#include "events/MouseScrollEvent.h"
#include "events/MouseMoveEvent.h"
#include "events/ResizeEvent.h"

#include "RHI/SwapChain.h"

namespace core
{
    using events::MouseMoveEvent;
    using events::MouseScrollEvent;
    using events::MouseButtonEvent;
    using events::ResizeEvent;

    Win32Window::Win32Window(HINSTANCE hInstance, int width, int height, const std::wstring& title, bool vSync)
        : _eventListeners{}
        , _vSync(vSync)
        , _title(title)
        , _fullscreen(false)
    {
        _windowStyle = WS_OVERLAPPEDWINDOW;

        _windowRect = { 0, 0, width, height };
        AdjustWindowRect(&_windowRect, _windowStyle, FALSE);

        _width = _windowRect.right - _windowRect.left;
        _height = _windowRect.bottom - _windowRect.top;

        _windowHandle = CreateWindowW(L"DX12WindowClass", title.c_str(),
            _windowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
            _width, _height, nullptr, nullptr, hInstance, this);

        _width = width;
        _height = height;

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

    void Win32Window::AddEventListener(events::IWindowEventListener* listener)
    {
        ASSERT(listener, "Trying to add a null event listener to the window.");
        _eventListeners.push_back(listener);
    }

    void Win32Window::RemoveEventListener(events::IWindowEventListener* listener)
    {
        auto it = std::find(_eventListeners.begin(), _eventListeners.end(), listener);

        if (it != _eventListeners.end())
        {
            _eventListeners.erase(it);
        }
        else
        {
            LOG_WARNING("Trying to remove a non-existing event listener from the window.");
        }
    }

    void Win32Window::SetSwapChain(rhi::SwapChain* swapChain)
    {
        FAIL(swapChain, "Trying to set a null swap chain to the window.");
        _swapChain = swapChain;
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
        ToggleFullscreenWindow();
    }

    void Win32Window::ToggleFullscreen()
    {
        SetFullscreen(!_fullscreen);
    }

    LRESULT Win32Window::WindowProcCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_SYSKEYDOWN:
        {
            // Handle ALT+ENTER:
            if ((wParam == VK_RETURN) && (lParam & (1 << 29)))
            {
                ToggleFullscreen();
            }
        }
        break;
        case WM_SIZE:
        {
            _width = ((int)(short)LOWORD(lParam));
            _height = ((int)(short)HIWORD(lParam));

            ResizeEvent resizeEventArgs(_width, _height);
            for (events::IWindowEventListener* listener : _eventListeners)
            {
                listener->OnResize(resizeEventArgs);
            }
        }
        break;
        case WM_DESTROY:
        {
            // If there are no more windows, quit the application.
            PostQuitMessage(0);
        }
        break;
        case WM_PIPELINE_CHANGED:
        {
            for (events::IWindowEventListener* listener : _eventListeners)
            {
                listener->OnPipelineChanged();
            }
        }
        break;
        case WM_LOAD_SCENE:
        {
            std::wstring wstr((WCHAR*)lParam);
            for (events::IWindowEventListener* listener : _eventListeners)
            {
                listener->OnLoadScene({ wstr.begin(), wstr.end() });
            }
            return 0;
        }
        break;
        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
        }

        return 0;
    }

    void Win32Window::ToggleFullscreenWindow()
    {
        if (_fullscreen)
        {
            // Save the old window rect so we can restore it when exiting fullscreen mode
            GetWindowRect(_windowHandle, &_windowRect);

            // Make the window borderless so that the client area can fill the screen
            SetWindowLong(_windowHandle, GWL_STYLE, _windowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

            rhi::ScissorRect fullscreenWindowRect;
            if (_swapChain)
            {
                // Get the settings of the display on which the app's window is currently displayed
                fullscreenWindowRect = _swapChain->GetDesktopCoordinates();
            }
            else
            {
                // Get the settings of the primary display
                DEVMODE devMode = {};
                devMode.dmSize = sizeof(DEVMODE);
                EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

                fullscreenWindowRect = {
                    devMode.dmPosition.x,
                    devMode.dmPosition.y,
                    devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
                    devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
                };
            }

            SetWindowPos(
                _windowHandle,
                HWND_TOPMOST,
                fullscreenWindowRect.Left,
                fullscreenWindowRect.Top,
                fullscreenWindowRect.Right,
                fullscreenWindowRect.Bottom,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ShowWindow(_windowHandle, SW_MAXIMIZE);
        }
        else
        {
            // Restore the window's attributes and size.
            SetWindowLong(_windowHandle, GWL_STYLE, _windowStyle);

            SetWindowPos(
                _windowHandle,
                HWND_NOTOPMOST,
                _windowRect.left,
                _windowRect.top,
                _windowRect.right - _windowRect.left,
                _windowRect.bottom - _windowRect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ShowWindow(_windowHandle, SW_NORMAL);
        }
    }
} // namespace core
