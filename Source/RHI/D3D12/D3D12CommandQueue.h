#pragma once

#include "CommandQueue.h"

namespace rhi
{
    class CommandList;
    class Device;
    class Fence;
} // namespace rhi

namespace rhi::d3d12
{
    class D3D12CommandQueue final : public rhi::CommandQueue
    {
    public:
        D3D12CommandQueue(const D3D12CommandQueue& other) = delete;
        D3D12CommandQueue(D3D12CommandQueue&& other) noexcept;
        ~D3D12CommandQueue() override = default;

        D3D12CommandQueue& operator=(const D3D12CommandQueue& other) = delete;
        D3D12CommandQueue& operator=(D3D12CommandQueue&& other) noexcept;

        void ExecuteCommandLists(std::vector<rhi::CommandList*> commandLists) override;

        void Signal(rhi::Fence* fence, std::uint64_t value) override;
        void Wait(rhi::Fence* fence, std::uint64_t value) override;

        std::uint64_t GetTimestampFrequency() const override;
        rhi::CommandListType GetType() const override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12CommandQueue(rhi::Device* device, rhi::CommandListType type, [[maybe_unused]] const std::string& name = "");

        ComPtr<ID3D12CommandQueue> _commandQueue;
        std::uint64_t _timestampFrequency;
        rhi::CommandListType _type;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
