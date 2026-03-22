#pragma once

#include "SwapChain.h"
#include "DescriptorHeap.h"

namespace rhi::d3d12
{
    // Number of back buffers in the swap chain (triple buffering).
    constexpr std::uint32_t BACK_BUFFER_COUNT = 3;

    // Wrapper for a DXGI swap chain, managing frame buffers and presentation.
    class D3D12SwapChain final : public rhi::SwapChain
    {
    public:
        // Copy constructor (deleted).
        D3D12SwapChain(const D3D12SwapChain& other) = delete;
        // Move constructor.
        D3D12SwapChain(D3D12SwapChain&& other) noexcept;
        // Destroys the swap chain and releases associated resources.
        ~D3D12SwapChain();

        // Copy assignment operator.
        D3D12SwapChain& operator=(const D3D12SwapChain& other) = delete;
        // Move assignment operator.
        D3D12SwapChain& operator=(D3D12SwapChain&& other) noexcept;

        // Initializes the swap chain for a given Win32 window.
        void Init(HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync = false);

        // Retrieves the swap chain description.
        DXGI_SWAP_CHAIN_DESC GetDescription() const;

        // Returns a pointer to the swap chain buffer at the specified index.
        std::shared_ptr<rhi::Texture> GetBuffer(std::uint32_t index);
        // Returns a pointer to the current back buffer.
        std::shared_ptr<rhi::Texture> GetBackBuffer();

        // Updates render target views for all back buffers.
        void UpdateRenderTargetViews();
        // Presents the current back buffer to the screen and returns the new back buffer index.
        std::uint32_t Present();

        // Handles swap chain resizing when the window size changes.
        void OnResize(const DirectX::XMUINT2& size);

        // Retrieves the output (monitor) that contains the swap chain window.
        ComPtr<IDXGIOutput> GetContainingOutput();

    private:
        friend class D3D12Device;

        // Constructs an uninitialized swap chain.
        D3D12SwapChain(rhi::Device* device);

        // Creates and configures the DXGI swap chain.
        ComPtr<IDXGISwapChain4> CreateSwapChain();
        // Checks if variable refresh rate (tearing support) is available.
        bool CheckTearingSupport() const;

        // Pointer to the DXGI swap chain interface.
        ComPtr<IDXGISwapChain4> _dxgiSwapChain;
        rhi::Device* _device;

        // Descriptor heap for render target views (RTVs).
        std::unique_ptr<DescriptorHeap> _RTVDescriptorHeap;
        // Size of an RTV descriptor in bytes.
        std::uint32_t _RTVDescriptorSize;

        // Array of back buffers managed by the swap chain.
        std::shared_ptr<rhi::Texture> _backBuffers[BACK_BUFFER_COUNT];
        // Index of the current back buffer being rendered to.
        std::uint32_t _currentBackBufferIndex;

        // Handle to the window associated with the swap chain.
        HWND _windowHandle;
        // Width of the swap chain buffers.
        std::uint32_t _width;
        // Height of the swap chain buffers.
        std::uint32_t _height;

        // Enables or disables vertical synchronization (VSync).
        bool _vSync;
        // Indicates whether the system supports tearing (variable refresh rate).
        bool _tearingSupport;
    };
} // namespace rhi::d3d12
