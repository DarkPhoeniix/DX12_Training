#pragma once

#include "StatisticsQuery.h"

namespace Core::Events
{
    class UpdateEvent;
}

class DebugInfo
{
public:
    static void Init();
    static void Destroy();

    static void Update(Core::Events::UpdateEvent& updateEvent);
    
    static void StartStatCollecting(dx12::CommandList& commandList);
    static void EndStatCollecting(dx12::CommandList& commandList);

    static const D3D12_QUERY_DATA_PIPELINE_STATISTICS& GetPipelineStatisctics();
    static UINT GetFPS();
    static double GetMsPerFrame();

private:
    DebugInfo();
    ~DebugInfo();

    static DebugInfo& Instance();

    dx12::StatisticsQuery _statisticsQuery;
    int _fps;
    double _msPerFrame;

    static DebugInfo* _instance;
};
