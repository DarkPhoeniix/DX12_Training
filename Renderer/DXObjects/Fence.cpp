#include "stdafx.h"

#include "Fence.h"

namespace Core
{
    Fence::Fence()
        : _fence(nullptr)
        , _fenceValue(0)
        , _isFree(true)
        , _eventOnCompletion{}
    {
    }

    Fence::~Fence()
    {
        _fence = nullptr;
    }

    void Fence::Init()
    {
        _fenceValue = 0;

        Core::Device::GetDXDevice()->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
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

    ComPtr<ID3D12Fence> Fence::GetFence()
    {
        return _fence;
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
} // namespace Core
