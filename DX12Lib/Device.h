#pragma once

#include "SwapChain.h"

namespace dx12
{
    class DescriptorHeap;

    // The Device class encapsulates the DirectX 12 device, adapters, command queues, and swap chain.
    class Device
    {
    public:
        // Delete copy constructor to enforce singleton pattern.
        Device(const Device& other) = delete;
        // Move constructor.
        Device(Device&& other) noexcept;

        // Delete copy assignment to enforce singleton pattern.
        Device& operator=(const Device& other) = delete;
        // Move assignment operator.
        Device& operator=(Device&& other) noexcept;

        // Initializes the DirectX 12 device and necessary resources.
        static void Init();
        // Cleans up and releases the DirectX 12 device resources.
        static void Destroy();

        // Binds a swap chain to the device for rendering output.
        static void BindSwapChain(SwapChain* swapChain);

        // Retrieves the underlying DirectX 12 device instance.
        static ComPtr<ID3D12Device2> GetDXDevice();
        // Retrieves the DXGI adapter associated with the device.
        static ComPtr<IDXGIAdapter4> GetDXAdapter();

        // Retrieves the compute command queue for GPU compute operations.
        static ID3D12CommandQueue* GetComputeQueue();
        // Retrieves the stream command queue for resource streaming.
        static ID3D12CommandQueue* GetStreamQueue();
        // Retrieves the copy command queue for efficient resource copying.
        static ID3D12CommandQueue* GetCopyQueue();

        // Handles resizing events by updating necessary resources.
        static void OnResize(const DirectX::XMUINT2& size);
        // Gets the current back buffer resource from the swap chain.
        static std::shared_ptr<dx12::Resource> GetBackBuffer();

        // Presents the rendered frame to the screen.
        static void Present();

        // Creates a Render Target View (RTV) in the specified descriptor heap.
        static void CreateRenderTargetView(const RenderTargetView& view, DescriptorHeap& descriptorHeap);
        // Creates a Depth Stencil View (DSV) in the specified descriptor heap.
        static void CreateDepthStencilView(const DepthStencilView& view, DescriptorHeap& descriptorHeap);
        // Creates a Constant Buffer View (CBV) in the specified descriptor heap.
        static void CreateConstantBufferView(const ConstantBufferView& view, DescriptorHeap& descriptorHeap);
        // Creates a Shader Resource View (SRV) in the specified descriptor heap.
        static void CreateShaderResourceView(const ShaderResourceView& view, DescriptorHeap& descriptorHeap);
        // Creates an Unordered Access View (UAV) in the specified descriptor heap.
        static void CreateUnorderedAccessView(const UnorderedAccessView& view, DescriptorHeap& descriptorHeap, std::shared_ptr<Resource> counterResource = nullptr);

    private:
        Device();
        ~Device();

        // Creates a DXGI adapter, optionally using WARP (software rasterizer).
        void CreateAdapter(bool userWarp = false);
        // Initializes the DirectX 12 device.
        void CreateDevice();
        // Creates the command queues.
        void CreateQueues();

        // DirectX 12 device.
        ComPtr<ID3D12Device2> _device;
        // DirectX 12 adapter.
        ComPtr<IDXGIAdapter4> _adapter;

        ComPtr<ID3D12CommandQueue> _queueCompute;
        ComPtr<ID3D12CommandQueue> _queueStream;
        ComPtr<ID3D12CommandQueue> _queueCopy;

        // Pointer to the swap chain bound to the device.
        SwapChain* _swapChain;

        // Singleton instance of the Device class.
        static Device* _instance;   // TODO: std::unique_ptr
    };
} // namespace dx12
