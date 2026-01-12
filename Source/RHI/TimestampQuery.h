#pragma once

namespace dx12
{
    class CommandList;
    class Resource;

    class TimestampQuery
    {
    public:
        TimestampQuery(std::uint32_t timestampsCount);
        ~TimestampQuery() = default;

        void Begin(CommandList& commandList, std::uint32_t index);
        void End(CommandList& commandList, std::uint32_t index);

        void Resolve(CommandList& commandList, std::uint32_t numTimestamps, std::shared_ptr<Resource> destinationBuffer, std::uint64_t destinationOffset);

        std::uint64_t GetFrequency() const;

    private:
        ComPtr<ID3D12QueryHeap> _queryHeap;

        std::uint64_t _frequency;
    };
} // namespace dx12