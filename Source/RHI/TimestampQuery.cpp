#include "RHI_PCH.h"

#include "TimestampQuery.h"

#include "CommandList.h"

namespace dx12
{
    TimestampQuery::TimestampQuery(std::uint32_t timestampsCount)
    {
        // Create query heap
        D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
        queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        queryHeapDesc.Count = timestampsCount;
        queryHeapDesc.NodeMask = 0;
        Device::GetDXDevice()->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&_queryHeap));

        // Get timestamp frequency
        Device::GetStreamQueue()->GetTimestampFrequency(&_frequency);
    }

    void TimestampQuery::Begin(CommandList& commandList, std::uint32_t index)
    {
        commandList.EndQuery(_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, static_cast<UINT>(index));
    }

    void TimestampQuery::End(CommandList& commandList, std::uint32_t index)
    {
        commandList.EndQuery(_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, static_cast<UINT>(index));
    }

    void TimestampQuery::Resolve(CommandList& commandList, std::uint32_t numTimestamps, std::shared_ptr<Resource> destinationBuffer, std::uint64_t destinationOffset)
    {
        commandList.ResolveQueryData(_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, numTimestamps, destinationBuffer, destinationOffset);
    }

    std::uint64_t TimestampQuery::GetFrequency() const
    {
        return _frequency;
    }
} // namespace dx12
