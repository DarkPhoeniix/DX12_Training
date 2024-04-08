#include "stdafx.h"
#include "Win32Window.h"

namespace Core
{
    Win32Window::Win32Window(HWND windowHandle, int width, int height, const std::wstring& title, bool vSync)
        : IWindow(width, height, title)
        , _windowHandle(windowHandle)
        , _vSync(vSync)
    {   }

    void Win32Window::Show()
    {
        ::ShowWindow(_windowHandle, SW_SHOW);
    }

    void Win32Window::Hide()
    {
        ::ShowWindow(_windowHandle, SW_HIDE);
    }

    std::shared_ptr<Win32Window> Win32Window::CreateWin32Window(int width, int height, const std::wstring& title, bool vSync)
    {
        RECT windowRect = { 0, 0, width, height };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        HWND hWnd = CreateWindowW(L"DX12WindowClass", title.c_str(),
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr, nullptr, nullptr, nullptr);

        if (!hWnd)
        {
            MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        std::shared_ptr<Win32Window> pWindow = std::make_shared<Win32Window>(hWnd, width, height, title, vSync);

        return pWindow;
    }

    HWND Win32Window::GetWindowHandle() const
    {
        return _windowHandle;
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
}
