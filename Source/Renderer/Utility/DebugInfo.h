#pragma once

#include "StatisticsQuery.h"
#include "HighResolutionClock.h"

namespace core::events
{
    class UpdateEvent;
    class RenderEvent;
}

class DebugInfo
{
public:
    static void Init();
    static void Destroy();

    static void BeginUpdate(core::events::UpdateEvent& updateEvent);
    static void EndUpdate();

    static void BeginRender(core::events::RenderEvent& renderEvent);
    static void EndRender();

    static void StartStatCollecting(dx12::CommandList& commandList);
    static void EndStatCollecting(dx12::CommandList& commandList);

    static const D3D12_QUERY_DATA_PIPELINE_STATISTICS& GetPipelineStatisctics();
    static UINT GetFPS();
    static double GetMsPerFrame();
    static double GetUpdateCPUTime();
    static double GetRenderCPUTime();

private:
    DebugInfo();
    ~DebugInfo();

    static DebugInfo& Instance();

    dx12::StatisticsQuery _statisticsQuery;

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
