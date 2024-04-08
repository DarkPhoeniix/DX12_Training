#pragma once

namespace Core
{
    class Device
    {
    public:
        Device(const Device& copy) = delete;
        Device operator=(const Device& copy) = delete;

        static ID3D12Device2* GetDevice();

    private:
        Device();
        ~Device();

        ID3D12Device2* GetDXDevice();

        void CreateDevice();
        void CreateAdapter(bool userWarp = false);

        ComPtr<ID3D12Device2> _device;
        ComPtr<IDXGIAdapter4> _adapter;
    };
} // namespace Core
