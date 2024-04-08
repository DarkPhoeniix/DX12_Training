#pragma once

#include "IWindow.h"

namespace Core
{
    class Win32Window : public IWindow
    {
    public:
        Win32Window(HWND windowHandle, int width, int height, const std::wstring& title, bool vSync = false);
        virtual ~Win32Window() = default;

        virtual void OnInit() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnRender() = 0;
        virtual void OnDestroy() = 0;

        HWND GetWindowHandle() const;

        bool IsVSync() const;
        void SetVSync(bool vSync);
        void ToggleVSync();

        bool IsFullScreen() const;
        void SetFullscreen(bool fullscreen);
        void ToggleFullscreen();

        void Show();
        void Hide();

        static std::shared_ptr<Win32Window> CreateWin32Window(int width, int height, const std::wstring& title, bool vSync);

    private:
        HWND _windowHandle;

        bool _vSync;
        bool _fullscreen;
        RECT _windowRect;
        bool _isTearingSupported;
    };
}
