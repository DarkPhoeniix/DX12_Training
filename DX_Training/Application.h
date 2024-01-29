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

    Microsoft::WRL::ComPtr<ID3D12Device2> getDevice() const;
    std::shared_ptr<CommandQueue> getCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

    void flush();

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
    UINT getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

protected:
    Application(HINSTANCE hIntance);
    virtual ~Application();

    Microsoft::WRL::ComPtr<ID3D12Device2> createDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
    Microsoft::WRL::ComPtr<IDXGIAdapter4> getAdapter(bool bUseWarp);

    bool checkTearingSupport();

private:
    // The application instance handle that this application was created with.
    HINSTANCE _hInstance;

    Microsoft::WRL::ComPtr<ID3D12Device2> _d3d12Device;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> _dxgiAdapter;

    std::shared_ptr<CommandQueue> _directCommandQueue;
    std::shared_ptr<CommandQueue> _computeCommandQueue;
    std::shared_ptr<CommandQueue> _copyCommandQueue;

    bool _isTearingSupported;
};
