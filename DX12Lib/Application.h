#pragma once

class Window;
class IGame;

class Application
{
public:
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    std::shared_ptr<Window> CreateWin32Window(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync = true);
    void DestroyWindow(const std::wstring& windowName);
    void DestroyWindow(std::shared_ptr<Window> window);

    std::shared_ptr<Window> getWindowByName(const std::wstring& windowName);

    int run(std::shared_ptr<IGame> pGame);
    void quit(int exitCode = 0);

    void flush();

    static void Init(HINSTANCE hInstance);
    static void Destroy();

    static Application& get();

protected:
    Application(HINSTANCE hIntance);
    virtual ~Application();

private:
    // The application instance handle that this application was created with.
    HINSTANCE _hInstance;
};
