#pragma once

class Win32Window;
class DXRenderer;

class Application
{
public:
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void Init(HINSTANCE hInstance);
    int Run(std::shared_ptr<DXRenderer> pApp);
    void Quit(int exitCode = 0);

    static Application& Get();

    static std::shared_ptr<Win32Window> CreateWin32Window(int width, int height, const std::wstring& title, bool vSync);

private:
    Application(HINSTANCE hInstance);
    ~Application();

    friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // The application instance handle that this application was created with.
    HINSTANCE _hInstance;

    static Application* _appInstance;
};
