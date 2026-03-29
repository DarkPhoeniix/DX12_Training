#pragma once

#include "TimestampQuery.h"

namespace rhi
{
    class Buffer;
    class CommandList;
    class QueryHeap;
} // namespace rhi

namespace rhi::d3d12
{
    class D3D12TimestampQuery final : public rhi::TimestampQuery
    {
    public:
        D3D12TimestampQuery(const D3D12TimestampQuery& other) = delete;
        D3D12TimestampQuery(D3D12TimestampQuery&& other) noexcept;
        ~D3D12TimestampQuery() override = default;

        D3D12TimestampQuery& operator=(const D3D12TimestampQuery& other) = delete;
        D3D12TimestampQuery& operator=(D3D12TimestampQuery&& other) noexcept;

        void Begin(rhi::CommandList* commandList, std::uint32_t index) override;
        void End(rhi::CommandList* commandList, std::uint32_t index) override;

        void Resolve(CommandList* commandList, std::uint32_t numTimestamps, std::shared_ptr<rhi::Buffer> destinationBuffer, std::uint64_t destinationOffset) override;

        std::uint64_t GetFrequency() const override;

    private:
        friend class D3D12Device;

        D3D12TimestampQuery(rhi::Device* device, std::uint32_t timestampsCount, const std::string& name = "");

        std::unique_ptr<rhi::QueryHeap> _queryHeap;
        std::uint64_t _frequency;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12