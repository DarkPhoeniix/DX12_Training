#pragma once

#include "SwapChain.h"

namespace rhi
{
    class DescriptorHeap;
} // namespace rhi

namespace rhi::d3d12
{
    // Wrapper for a DXGI swap chain, managing frame buffers and presentation.
    class D3D12SwapChain final : public rhi::SwapChain
    {
    public:
        D3D12SwapChain(const D3D12SwapChain& other) = delete;
        D3D12SwapChain(D3D12SwapChain&& other) noexcept;
        ~D3D12SwapChain() override;

        D3D12SwapChain& operator=(const D3D12SwapChain& other) = delete;
        D3D12SwapChain& operator=(D3D12SwapChain&& other) noexcept;

        std::shared_ptr<rhi::Texture> GetBuffer(std::uint32_t index) override;
        std::shared_ptr<rhi::Texture> GetBackBuffer() override;

        // Updates render target views for all back buffers.
        void UpdateRenderTargetViews() override;
        // Presents the current back buffer to the screen and returns the new back buffer index.
        std::uint32_t Present() override;

        void OnResize(std::uint32_t width, std::uint32_t height) override;

        rhi::ScissorRect GetDesktopCoordinates() override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12SwapChain(rhi::Device* device, HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync);

        ComPtr<IDXGISwapChain4> CreateSwapChain();
        bool CheckTearingSupport() const;

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
