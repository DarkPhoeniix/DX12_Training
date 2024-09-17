#include "stdafx.h"

#include "DebugInfo.h"

#include "Events/UpdateEvent.h"

DebugInfo* DebugInfo::_instance = nullptr;

void DebugInfo::Init()
{
    if (!_instance)
    {
        _instance = new DebugInfo;
    }
}

void DebugInfo::Destroy()
{
    if (_instance)
    {
        delete _instance;
    }
    _instance = nullptr;
}

void DebugInfo::Update(Core::Events::UpdateEvent& updateEvent)
{
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;

    totalTime += updateEvent.elapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        Instance()._fps = (int)(frameCount / totalTime);
        Instance()._msPerFrame = totalTime / frameCount * 1000;

        frameCount = 0;
        totalTime = 0.0;
    }
}

void DebugInfo::StartStatCollecting(Core::GraphicsCommandList& commandList)
{
    Instance()._statisticsQuery.BeginQuery(commandList);
}

void DebugInfo::EndStatCollecting(Core::GraphicsCommandList& commandList)
{
    Instance()._statisticsQuery.EndQuery(commandList);
    Instance()._statisticsQuery.ResolveQueryData(commandList);
}

const D3D12_QUERY_DATA_PIPELINE_STATISTICS& DebugInfo::GetPipelineStatisctics()
{
    return Instance()._statisticsQuery.GetStatistics();
}

UINT DebugInfo::GetFPS()
{
    return Instance()._fps;
}

double DebugInfo::GetMsPerFrame()
{
    return Instance()._msPerFrame;
}

DebugInfo::DebugInfo()
    : _fps(0)
    , _msPerFrame(0)
{
    _statisticsQuery.Create();
}

DebugInfo::~DebugInfo()
{
}

DebugInfo& DebugInfo::Instance()
{
    ASSERT(_instance, "DebugInfo instance not created");
    return *_instance;
}
