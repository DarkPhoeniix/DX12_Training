#pragma once

#include "RHI/TimestampQuery.h"
#include "Utility/Timer.h"

class Profiler
{
public:
    using TimerID = std::uint16_t;
    static const TimerID InvalidTimerID = static_cast<TimerID>(-1);

    struct TimerResult
    {
        TimerID ID = InvalidTimerID;
        double TimeMs = 0.0;
    };

    struct Stats
    {
        std::uint64_t FrameID = std::uint64_t(-1);
        float FrameTimeMs = 0.0f;

        std::vector<TimerResult> TimerResults = {};
    };

    Profiler(rhi::Device* device);
    ~Profiler() = default;

    void BeginEvent(rhi::CommandList* commandList, TimerID id);
    void EndEvent(rhi::CommandList* commandList, TimerID id);

    TimerID RegisterTimer(const std::string& name);
    void UnregisterAllTimers();

    void ResolveTimestamps(rhi::CommandList* commandList);

    const Stats& GetCPUStats() const;
    const Stats& GetGPUStats() const;

    const std::string& GetTimerName(TimerID id) const;

private:
    struct TimerInfo
    {
        std::string name;
        TimerID id;
    };
    std::vector<TimerInfo> _timers;

    // CPU statistics
    Stats _cpuStats;

    std::vector<Timer> _cpuTimers;

    // GPU statistics
    Stats _gpuStats;

    std::unique_ptr<rhi::TimestampQuery> _timestampQuery;
    std::shared_ptr<rhi::Buffer> _timestampResultBuffer;
};
