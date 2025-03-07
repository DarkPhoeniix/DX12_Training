#pragma once

#include "CommandList.h"

namespace dx12
{
    // Wrapper for an ID3D12QueryHeap to gather GPU time statistics.
    class TimestampQuery
    {
    public:
        void Create(std::uint32_t numTimers);

        void QueryTimestamp(CommandList& commandList, std::uint32_t timerId);

        void ResolveQueryData(CommandList& commandList);
        std::uint64_t GetStatistics(std::uint32_t timerId);

    private:
        ComPtr<ID3D12QueryHeap> _timestampQueryHeap;

        dx12::Resource _statisticsResource;
        std::uint32_t _numTimers;
        std::uint64_t* _timeData;
    };
} // namespace dx12
