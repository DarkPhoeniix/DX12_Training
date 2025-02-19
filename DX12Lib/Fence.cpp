#include "DX12LibPCH.h"

#include "Fence.h"

namespace dx12
{
    void Fence::Init()
    {
        _fenceValue = 0;

        Device::GetDXDevice()->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
        _eventOnCompletion = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    void Fence::Wait()
    {
        if (_fence->GetCompletedValue() >= _fenceValue)
        {
            return;
        }

        this->_fence->SetEventOnCompletion(_fenceValue, _eventOnCompletion);
        ::WaitForSingleObject(_eventOnCompletion, DWORD_MAX);
    }

    void Fence::SetValue(UINT64 fenceValue)
    {
        _fenceValue = fenceValue;
    }

    UINT64 Fence::GetValue() const
    {
        return _fenceValue;
    }

    void Fence::SetFree(bool isFree)
    {
        _isFree = isFree;
    }

    bool Fence::IsFree() const
    {
        return _isFree;
    }

    ComPtr<ID3D12Fence> Fence::GetDXFence()
    {
        return _fence;
    }
} // namespace dx12
