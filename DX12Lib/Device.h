#pragma once

#include "SwapChain.h"

namespace dx12
{
    class Device
    {
    public:
        Device(const Device& copy) = delete;
        Device operator=(const Device& copy) = delete;

        static void Init();
        static void Destroy();

        static void BindSwapChain(SwapChain* swapChain);

        static ComPtr<ID3D12Device2> GetDXDevice();

        static ID3D12CommandQueue* GetComputeQueue();
        static ID3D12CommandQueue* GetStreamQueue();
        static ID3D12CommandQueue* GetCopyQueue();

        static void OnResize(const DirectX::XMUINT2& size);
        static Resource* GetBackBuffer();

        static void Present();

    private:
        Device();
        ~Device();

        void CreateAdapter(bool userWarp = false);
        void CreateDevice();
        void CreateQueues();

        ComPtr<ID3D12Device2> _device;
        ComPtr<IDXGIAdapter4> _adapter;

        ComPtr<ID3D12CommandQueue> _queueCompute;
        ComPtr<ID3D12CommandQueue> _queueStream;
        ComPtr<ID3D12CommandQueue> _queueCopy;

        SwapChain* _swapChain;

        static Device* _instance;
    };
} // namespace dx12
