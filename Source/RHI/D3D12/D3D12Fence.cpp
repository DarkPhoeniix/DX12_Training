
#include "RHI_PCH.h"

#include "D3D12Fence.h"

#include "D3D12Device.h"
#include "D3D12Helpers.h"

namespace rhi::d3d12
{
    D3D12Fence::D3D12Fence(Device* device, std::uint64_t initialValue)
        : _eventOnCompletion(nullptr)
        , _fenceValue(initialValue)
        , _cpuCallback()
        , _isFree(true)
    {
        NativeDevice* d3d12NativeDevice = D3D12Cast<NativeDevice>(device->GetNative());
        d3d12NativeDevice->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
    }

    D3D12Fence::D3D12Fence(D3D12Fence&& other) noexcept
        : _fence(std::move(other._fence))
        , _eventOnCompletion(other._eventOnCompletion)
        , _cpuCallback(std::move(other._cpuCallback))
        , _fenceValue(other._fenceValue)
        , _isFree(other._isFree)
    {
    }

    D3D12Fence::~D3D12Fence()
    {
    }

    D3D12Fence& D3D12Fence::operator=(D3D12Fence&& other) noexcept
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

    void D3D12Fence::Wait()
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

    void* D3D12Fence::GetNative() const
    {
        return static_cast<void*>(_fence.Get());
    }
} // namespace rhi::d3d12
