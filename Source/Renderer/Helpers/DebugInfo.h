#pragma once

#include "RHI/StatisticsQuery.h"
#include "Utility/HighResolutionClock.h"

namespace core::events
{
    class UpdateEvent;
    class RenderEvent;
} // namespace core::events

class DebugInfo
{
public:
    static void Init(rhi::Device* device);
    static void Destroy();

    static void BeginUpdate(core::events::UpdateEvent& updateEvent);
    static void EndUpdate();

    static void BeginRender(core::events::RenderEvent& renderEvent);
    static void EndRender();

    static void StartStatCollecting(rhi::CommandList* commandList);
    static void EndStatCollecting(rhi::CommandList* commandList);

    static const rhi::PipelineStatistics& GetPipelineStatisctics();
    static UINT GetFPS();
    static double GetMsPerFrame();
    static double GetUpdateCPUTime();
    static double GetRenderCPUTime();

private:
    DebugInfo(rhi::Device* device);
    ~DebugInfo();

    static DebugInfo& Instance();

    std::unique_ptr<rhi::StatisticsQuery> _statisticsQuery;

    HighResolutionClock _frameTimer;
    std::uint32_t _fps;
    double _msPerFrame;

    std::chrono::high_resolution_clock::time_point _startUpdateTime;
    std::uint64_t _totalUpdateFrames;
    double _totalUpdateTime;
    double _updateTime;

    std::chrono::high_resolution_clock::time_point _startRenderTime;
    std::uint64_t _totalRenderFrames;
    double _totalRenderTime;
    double _renderTime;

    static DebugInfo* _instance;
};
