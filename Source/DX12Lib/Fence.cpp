#include "DX12LibPCH.h"

#include "Fence.h"

namespace dx12
{
    Fence::Fence()
        : _fence(nullptr)
        , _eventOnCompletion(nullptr)
        , _fenceValue(0)
        , _cpuCallback()
        , _isFree(true)
    {
    }

    Fence::Fence(const Fence& other)
        : _fence(other._fence)
        , _eventOnCompletion(other._eventOnCompletion)
        , _cpuCallback(other._cpuCallback)
        , _fenceValue(other._fenceValue)
        , _isFree(other._isFree)
    {
    }

    Fence::Fence(Fence&& other) noexcept
        : _fence(std::move(other._fence))
        , _eventOnCompletion(other._eventOnCompletion)
        , _cpuCallback(std::move(other._cpuCallback))
        , _fenceValue(other._fenceValue)
        , _isFree(other._isFree)
    {
    }

    Fence::~Fence()
    {
        _fence = nullptr;
    }

    Fence& Fence::operator=(const Fence& other)
    {
        if (this != &other)
        {
            _fence = other._fence;
            _eventOnCompletion = other._eventOnCompletion;
            _cpuCallback = other._cpuCallback;
            _fenceValue = other._fenceValue;
            _isFree = other._isFree;
        }

        return *this;
    }

    Fence& Fence::operator=(Fence&& other) noexcept
    {
        if (this != &other)
        {
            _fence = std::move(other._fence);
            _eventOnCompletion = other._eventOnCompletion;
            _cpuCallback = std::move(other._cpuCallback);
            _fenceValue = other._fenceValue;
            _isFree = other._isFree;
        }

        return *this;
    }

    void Fence::Init()
    {
        _fenceValue = 0;

        HRESULT result = Device::GetDXDevice()->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
        CHECK(result, "Failed to create fence.");

        _eventOnCompletion = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    void Fence::Wait()
    {
        if (_fence->GetCompletedValue() >= _fenceValue)
        {
            return;
        }

        HRESULT result = this->_fence->SetEventOnCompletion(_fenceValue, _eventOnCompletion);
        CHECK(result, "Failed to set event on fence completion.");
        ::WaitForSingleObject(_eventOnCompletion, DWORD_MAX);

        if (_cpuCallback)
        {
            _cpuCallback();
            _cpuCallback = {};
        }
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

    void Fence::SetCompletionCallback(const std::function<void()>& callback)
    {
        _cpuCallback = callback;
    }
} // namespace dx12
