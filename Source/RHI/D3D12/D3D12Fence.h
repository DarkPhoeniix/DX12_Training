#pragma once

#include "Fence.h"

#include <functional>

namespace rhi::d3d12
{
    // Wrapper for an ID3D12Fence object to synchronize the CPU and GPU.
    class D3D12Fence final : public rhi::Fence
    {
    public:
        // Copy constructor.
        D3D12Fence(const D3D12Fence& other) = delete;
        // Move constructor.
        D3D12Fence(D3D12Fence&& other) noexcept;
        // Destructor.
        ~D3D12Fence() override;

        // Copy assignment operator.
        D3D12Fence& operator=(const D3D12Fence& other) = delete;
        // Move assignment operator.
        D3D12Fence& operator=(D3D12Fence&& other) noexcept;

        // Waits until the fence reaches the current value.
        void Wait() override;

        // Inherited via Fence
        void SetValue(std::uint64_t value) override { _fenceValue = value; }
        std::uint64_t GetValue() const override { return _fenceValue; }

        void SetFree(bool isFree) override { _isFree = isFree; }
        bool IsFree() const override { return _isFree; }

        void SetCompletionCallback(const std::function<void()>& callback) override { _cpuCallback = callback; }

        // Get a pointer to the raw D3D12 fence object.
        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12Fence(rhi::Device* device, std::uint64_t initialValue);

        // Raw D3D12 fence object.
        ComPtr<ID3D12Fence> _fence;
        // Event triggered when _fence reaches _fenceValue.
        HANDLE _eventOnCompletion;

        std::uint64_t _fenceValue;
        bool _isFree;

        std::function<void()> _cpuCallback;
    };
} // namespace rhi::d3d12
