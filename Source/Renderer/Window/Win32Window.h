#pragma once

#include "SwapChain.h"
#include "Window/IWindowEventListener.h"

namespace core
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

        void AddEventListener(events::IWindowEventListener* listener);
        void RemoveEventListener(events::IWindowEventListener* listener);

        void SetSwapChain(dx12::SwapChain* swapChain);

        LRESULT WindowProcCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    protected:
        void ToggleFullscreenWindow();

        HWND _windowHandle;

        std::vector<events::IWindowEventListener*> _eventListeners;

        dx12::SwapChain* _swapChain;

        std::wstring _title;
        UINT _windowStyle;
        RECT _windowRect;
        int _width;
        int _height;

        bool _vSync;
        bool _fullscreen;
    };
} // namespace core
