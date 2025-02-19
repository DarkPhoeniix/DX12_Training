#pragma once

namespace dx12
{
    // Wrapper for an ID3D12Fence object to synchronize the CPU and GPU.
    class Fence
    {
    public:
        // Initializes the fence and marks it as free.
        void Init();

        // Waits until the fence reaches the current value.
        void Wait();

        // Set the fence value.
        void SetValue(UINT64 fenceValue);
        // Get the current fence value.
        UINT64 GetValue() const;

        // Set whether the fence is free.
        void SetFree(bool isFree);
        // Check if the fence is free.
        bool IsFree() const;

        // Get a pointer to the raw D3D12 fence object.
        ComPtr<ID3D12Fence> GetDXFence();

    private:
        // Raw D3D12 fence object.
        ComPtr<ID3D12Fence> _fence = nullptr;
        // Event triggered when _fence reaches _fenceValue.
        HANDLE _eventOnCompletion = nullptr;
        // Current fence value.
        UINT64 _fenceValue = 0;

        // Indicates whether the fence is available for use.
        bool _isFree = true;
    };
} // namespace dx12
