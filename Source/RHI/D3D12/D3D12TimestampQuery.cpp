
#include "RHI_PCH.h"

#include "D3D12TimestampQuery.h"

#include "CommandList.h"
#include "CommandQueue.h"

namespace rhi::d3d12
{
    D3D12TimestampQuery::D3D12TimestampQuery(rhi::Device* device, std::uint32_t timestampsCount, const std::string& name)
        : _frequency(device->GetStreamQueue()->GetTimestampFrequency())
    {
        QueryHeapDescription description =
        {
            .Type = rhi::QueryHeapType::Timestamp,
            .Count = timestampsCount,
            .NodeMask = 0
        };
        _queryHeap = device->CreateQueryHeap(description);
    }

    void D3D12TimestampQuery::Begin(CommandList& commandList, std::uint32_t index)
    {
        commandList.EndQuery(*_queryHeap, rhi::QueryType::Timestamp, index);
    }

    void D3D12TimestampQuery::End(CommandList& commandList, std::uint32_t index)
    {
        commandList.EndQuery(*_queryHeap, rhi::QueryType::Timestamp, index);
    }

    void D3D12TimestampQuery::Resolve(CommandList& commandList, std::uint32_t numTimestamps, std::shared_ptr<rhi::Buffer> destinationBuffer, std::uint64_t destinationOffset)
    {
        commandList.ResolveQueryData(*_queryHeap, rhi::QueryType::Timestamp, 0, numTimestamps, destinationBuffer, destinationOffset);
    }

    std::uint64_t D3D12TimestampQuery::GetFrequency() const
    {
        return _frequency;
    }
} // namespace rhi::d3d12
