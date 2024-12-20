#pragma once

#include "Window/IWindowEventListener.h"

namespace Core
{
    class Win32Window
    {
    public:
        Win32Window(HINSTANCE hInstance, int width, int height, const std::wstring& title, bool vSync = false);
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

        void AddEventListener(Events::IWindowEventListener* listener);
        void RemoveEventListener(Events::IWindowEventListener* listener);

        LRESULT WindowProcCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    protected:
        HWND _windowHandle;

        std::vector<Events::IWindowEventListener*> _eventListeners;

        std::wstring _title;
        int _width;
        int _height;

        bool _vSync;
        bool _fullscreen;
    };
} // namespace Core
