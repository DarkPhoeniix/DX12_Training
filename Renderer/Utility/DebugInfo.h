#pragma once

#include "DXObjects/StatisticsQuery.h"
#include "Utility/HighResolutionClock.h"

namespace Core::Events
{
    class UpdateEvent;
}

class DebugInfo
{
public:
    static void Update(Core::Events::UpdateEvent& updateEvent);
    
    static void StartStatCollecting(Core::GraphicsCommandList& commandList);
    static void EndStatCollecting(Core::GraphicsCommandList& commandList);

    static const D3D12_QUERY_DATA_PIPELINE_STATISTICS& GetPipelineStatisctics();
    static UINT GetFPS();
    static double GetMsPerFrame();

private:
    DebugInfo();
    ~DebugInfo();

    static DebugInfo& Instance();

    Core::StatisticsQuery _statisticsQuery;
    int _fps;
    double _msPerFrame;
};
