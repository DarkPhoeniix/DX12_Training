#pragma once

struct Fence
{
public:
    void Init(ComPtr<ID3D12Device> device);

    void Wait();

    ComPtr<ID3D12Fence> GetFence();

    void SetValue(UINT64 fenceValue);
    UINT64 GetValue() const;

    void SetFree(bool isFree);
    bool IsFree() const;

private:
    ComPtr<ID3D12Fence> _fence;
    UINT64 _fenceValue = 0;
    HANDLE _eventOnCompletion;

    bool _isFree = true;
};
