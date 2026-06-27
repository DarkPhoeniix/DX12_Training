#pragma once

#include "SwapChain.h"

namespace rhi
{
    class DescriptorHeap;
} // namespace rhi

namespace rhi::d3d12
{
    // Wrapper for a DXGI swap chain, managing frame buffers and presentation.
    class D3D12SwapChain final : public SwapChain
    {
    public:
        D3D12SwapChain(const D3D12SwapChain& other) = delete;
        D3D12SwapChain(D3D12SwapChain&& other) noexcept;
        ~D3D12SwapChain() override;

        D3D12SwapChain& operator=(const D3D12SwapChain& other) = delete;
        D3D12SwapChain& operator=(D3D12SwapChain&& other) noexcept;

        std::shared_ptr<Texture> GetBuffer(std::uint32_t index) override;
        std::shared_ptr<Texture> GetBackBuffer() override;

        std::uint32_t Present() override;

        void OnResize(std::uint32_t width, std::uint32_t height) override;

        ScissorRect GetDesktopCoordinates() override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12SwapChain(Device* device, HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync);

        ComPtr<IDXGISwapChain4> CreateSwapChain();
        bool CheckTearingSupport() const;

        void UpdateRenderTargetViews();

        ComPtr<IDXGISwapChain4> _dxgiSwapChain;
        Device* _device;

        std::unique_ptr<DescriptorHeap> _RTVDescriptorHeap;
        std::uint32_t _RTVDescriptorSize;

        std::shared_ptr<Texture> _backBuffers[BACK_BUFFER_COUNT];
        std::uint32_t _currentBackBufferIndex;

        HWND _windowHandle;
        std::uint32_t _width;
        std::uint32_t _height;

        bool _vSync;
        bool _tearingSupport;
    };
} // namespace rhi::d3d12
