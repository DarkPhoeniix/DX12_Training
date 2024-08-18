#pragma once

namespace Core
{
    class Fence
    {
    public:
        Fence();
        ~Fence();

        void Init();

        void Wait();

        ComPtr<ID3D12Fence> GetFence();

        void SetValue(UINT64 fenceValue);
        UINT64 GetValue() const;

        void SetFree(bool isFree);
        bool IsFree() const;

    private:
        ComPtr<ID3D12Device2> _DXDevice;

        ComPtr<ID3D12Fence> _fence;
        UINT64 _fenceValue;
        HANDLE _eventOnCompletion;

        bool _isFree;
    };
} // namespace Core
