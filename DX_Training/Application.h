#pragma once

#include "Interfaces/IRenderWindow.h"

class RenderWindow;
class CommandQueue;

class Application
{
public:
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    std::shared_ptr<RenderWindow> createRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync = true);

    bool isTearingSupported() const;

    void flush();

    static void create(HINSTANCE hInstance);
    static void destroy();
    static Application& get();
    static int run(RenderWindow* window);

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

    Microsoft::WRL::ComPtr<IDXGIAdapter4> _dxgiAdapter;
    Microsoft::WRL::ComPtr<ID3D12Device2> _d3d12Device;

    std::shared_ptr<CommandQueue> _directCommandQueue;
    std::shared_ptr<CommandQueue> _computeCommandQueue;
    std::shared_ptr<CommandQueue> _copyCommandQueue;

    bool _isTearingSupported;
};
