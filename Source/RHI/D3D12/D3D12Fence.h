#pragma once

#include "Fence.h"

namespace rhi::d3d12
{
    class D3D12Fence final : public Fence
    {
    public:
        D3D12Fence(const D3D12Fence& other) = delete;
        D3D12Fence(D3D12Fence&& other) noexcept;
        ~D3D12Fence() override;

        D3D12Fence& operator=(const D3D12Fence& other) = delete;
        D3D12Fence& operator=(D3D12Fence&& other) noexcept;

        void Wait() override;

        void SetValue(std::uint64_t value) override { _fenceValue = value; }
        std::uint64_t GetValue() const override { return _fenceValue; }

        void SetFree(bool isFree) override { _isFree = isFree; }
        bool IsFree() const override { return _isFree; }

        void SetCompletionCallback(const std::function<void()>& callback) override { _cpuCallback = callback; }

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12Fence(Device* device, std::uint64_t initialValue);

        ComPtr<ID3D12Fence> _fence;
        HANDLE _eventOnCompletion;

        std::uint64_t _fenceValue;
        bool _isFree;

        std::function<void()> _cpuCallback;
    };
} // namespace rhi::d3d12
