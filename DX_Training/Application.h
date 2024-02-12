#pragma once

class Window;
class CommandQueue;
class IGame;

class Application
{
public:
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    std::shared_ptr<Window> createWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync = true);
    void destroyWindow(const std::wstring& windowName);
    void destroyWindow(std::shared_ptr<Window> window);

    std::shared_ptr<Window> getWindowByName(const std::wstring& windowName);

    bool isTearingSupported() const;

    static void create(HINSTANCE hInstance);
    static void destroy();
    static Application& get();
    int run(std::shared_ptr<IGame> pGame);
    void quit(int exitCode = 0);

    ComPtr<ID3D12Device2> getDevice() const;

    void flush();

    ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
    UINT getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

protected:
    Application(HINSTANCE hIntance);
    virtual ~Application();

    ComPtr<ID3D12Device2> createDevice(ComPtr<IDXGIAdapter4> adapter);
    ComPtr<IDXGIAdapter4> getAdapter(bool bUseWarp);

    bool checkTearingSupport();

private:
    // The application instance handle that this application was created with.
    HINSTANCE _hInstance;

    ComPtr<ID3D12Device2> _d3d12Device;
    ComPtr<IDXGIAdapter4> _dxgiAdapter;

    bool _isTearingSupported;
};
