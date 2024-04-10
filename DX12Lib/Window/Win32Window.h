#pragma once

#include "IWindowEventListener.h"

namespace Core
{
    class Win32Window
    {
    public:
        Win32Window(HWND windowHandle, int width, int height, const std::wstring& title, bool vSync = false);
        virtual ~Win32Window() = default;

        HWND GetWindowHandle() const;

        const std::wstring& GetTitle() const;
        int GetWidth() const;
        int GetHeight() const;

        bool IsVSync() const;
        void SetVSync(bool vSync);
        void ToggleVSync();

        bool IsFullScreen() const;
        void SetFullscreen(bool fullscreen);
        void ToggleFullscreen();

        void Show();
        void Hide();

        void AddEventListener(IWindowEventListener* listener);
        void RemoveEventListener();

        LRESULT WindowProcCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        static std::shared_ptr<Win32Window> CreateWin32Window(int width, int height, const std::wstring& title, bool vSync);

    protected:
        HWND _windowHandle;

        IWindowEventListener* _eventListener;

        std::wstring _title;
        int _width;
        int _height;

        bool _vSync;
        bool _fullscreen;
    };
}
