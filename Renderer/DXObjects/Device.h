#pragma once

namespace Core
{
    class Device
    {
    public:
        Device(const Device& copy) = delete;
        Device operator=(const Device& copy) = delete;

        static void Init();
        static void Destroy();

        static ComPtr<ID3D12Device2> GetDXDevice();

        static ID3D12CommandQueue* GetComputeQueue();
        static ID3D12CommandQueue* GetStreamQueue();
        static ID3D12CommandQueue* GetCopyQueue();

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

        static Device* _instance;
    };
} // namespace Core
