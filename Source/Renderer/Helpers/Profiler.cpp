#include "RendererPCH.h"

#include "Profiler.h"

namespace
{
    constexpr std::uint16_t MAX_TIMERS = 256;
}

Profiler::Profiler()
    : _timestampQuery(MAX_TIMERS)
{
    _gpuStats =
    {
        .FrameID = 0,
        .FrameTimeMs = 0.0f,
        .TimerResults = std::vector<TimerResult>(MAX_TIMERS)
    };

    _timestampQuery = dx12::TimestampQuery(MAX_TIMERS * 2);

    dx12::ResourceDescription bufferDesc;
    bufferDesc.SetSize({ sizeof(std::uint64_t) * MAX_TIMERS * 2, 1 });
    bufferDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::ReadBack);

    _timestampResultBuffer = ResourceFactory::Create("gpu_timestamp_result_buffer", bufferDesc);
    _timestampResultBuffer->CreateCommitedResource(dx12::ResourceState::CopyDest);
}

void Profiler::BeginEvent(dx12::CommandList& commandList, TimerID id)
{
    _cpuTimers[id].Start();
    _timestampQuery.Begin(commandList, id * 2);
}

void Profiler::EndEvent(dx12::CommandList& commandList, TimerID id)
{
    _cpuTimers[id].Stop();
    _timestampQuery.End(commandList, id * 2 + 1);
}

Profiler::TimerID Profiler::RegisterTimer(const std::string& name)
{
#if _DEBUG
    auto timerIt = std::find(_timers.begin(), _timers.end(), [&](const Profiler::TimerInfo& info) { return info.name == name; });
    ASSERT(timerIt == _timers.end(), "Profiler: Timer with name \'{}\' is already registered.", name);
#endif

    Profiler::TimerID id = static_cast<Profiler::TimerID>(_timers.size());
    _timers.emplace_back(name, id);
    _cpuTimers.emplace_back();
    return id;
}

void Profiler::UnregisterAllTimers()
{
    _timers.clear();
    _cpuTimers.clear();
    _cpuStats = {};
    _gpuStats = {};
}

void Profiler::ResolveTimestamps(dx12::CommandList& commandList)
{
    if (_timers.size() != _gpuStats.TimerResults.size())
    {
        _cpuStats.TimerResults.resize(_timers.size() - 1);
        _gpuStats.TimerResults.resize(_timers.size() - 1);
    }

    _timestampQuery.Resolve(commandList, _timers.size() * 2, _timestampResultBuffer, 0);

    _gpuStats.FrameID++;

    std::uint64_t* data = _timestampResultBuffer->Map<std::uint64_t>();
    size_t index = 0;
    for (const TimerInfo& timer : _timers)
    {
        std::uint64_t startTimestamp = data[timer.id * 2];
        std::uint64_t endTimestamp = data[timer.id * 2 + 1];

        double timeMs = double(endTimestamp - startTimestamp) * 1000.0 / static_cast<double>(_timestampQuery.GetFrequency());
        if (timer.name == "Frame")
        {
            _cpuStats.FrameTimeMs = _cpuTimers[timer.id].GetElapsedMilliseconds();
            _gpuStats.FrameTimeMs = static_cast<float>(timeMs);
        }
        else
        {
            _cpuStats.TimerResults[index] = { timer.id, _cpuTimers[timer.id].GetElapsedMilliseconds() };
            _gpuStats.TimerResults[index++] = { timer.id, timeMs };
        }
    }
}

const Profiler::Stats& Profiler::GetCPUStats() const
{
    return _cpuStats;
}

const Profiler::Stats& Profiler::GetGPUStats() const
{
    return _gpuStats;
}

const std::string& Profiler::GetTimerName(TimerID id) const
{
    return _timers[id].name;
}
