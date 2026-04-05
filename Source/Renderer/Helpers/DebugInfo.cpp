#include "RendererPCH.h"

#include "DebugInfo.h"

#include "Events/UpdateEvent.h"
#include "Events/RenderEvent.h"

namespace
{
    static constexpr double kUpdateInterval = 0.1;
}

DebugInfo* DebugInfo::_instance = nullptr;

void DebugInfo::Init(rhi::Device* device)
{
    if (!_instance)
    {
        _instance = new DebugInfo(device);
    }
    else
    {
        ASSERT(false, "DebugInfo instance already created");
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

void DebugInfo::BeginUpdate(core::events::UpdateEvent& updateEvent)
{
    Instance()._startUpdateTime = std::chrono::high_resolution_clock::now();
}

void DebugInfo::EndUpdate()
{
    double delta = static_cast<double>((std::chrono::high_resolution_clock::now() - Instance()._startUpdateTime).count()) * 1e-9;
    Instance()._totalUpdateTime += delta;
    Instance()._totalUpdateFrames++;

    if (Instance()._totalUpdateTime >= kUpdateInterval)
    {
        Instance()._updateTime = Instance()._totalUpdateTime / Instance()._totalUpdateFrames * 1000;
        Instance()._totalUpdateFrames = 0;
        Instance()._totalUpdateTime = 0.0;
    }
}

void DebugInfo::BeginRender(core::events::RenderEvent& renderEvent)
{
    Instance()._startRenderTime = std::chrono::high_resolution_clock::now();
    Instance()._startRenderTime = std::chrono::high_resolution_clock::now();
}

void DebugInfo::EndRender()
{
    Instance()._frameTimer.Tick();

    double delta = static_cast<double>((std::chrono::high_resolution_clock::now() - Instance()._startRenderTime).count()) * 1e-9;
    Instance()._totalRenderTime += delta;
    Instance()._totalRenderFrames++;

    if (Instance()._totalRenderTime >= kUpdateInterval)
    {
        Instance()._renderTime = Instance()._totalRenderTime / Instance()._totalRenderFrames * 1000;
        Instance()._totalRenderTime = 0.0;

        Instance()._msPerFrame = Instance()._frameTimer.GetTotalMilliSeconds() / Instance()._totalRenderFrames;
        Instance()._fps = static_cast<std::uint32_t>(1000.0 / Instance()._msPerFrame);
        Instance()._frameTimer.Reset();

        Instance()._totalRenderFrames = 0;
    }
}

void DebugInfo::StartStatCollecting(rhi::CommandList* commandList)
{
    Instance()._statisticsQuery->BeginQuery(commandList);
}

void DebugInfo::EndStatCollecting(rhi::CommandList* commandList)
{
    Instance()._statisticsQuery->EndQuery(commandList);
    Instance()._statisticsQuery->ResolveQueryData(commandList);
}

const rhi::PipelineStatistics& DebugInfo::GetPipelineStatisctics()
{
    return Instance()._statisticsQuery->GetStatistics();
}

UINT DebugInfo::GetFPS()
{
    return Instance()._fps;
}

double DebugInfo::GetMsPerFrame()
{
    return Instance()._msPerFrame;
}

double DebugInfo::GetUpdateCPUTime()
{
    return Instance()._updateTime;
}

double DebugInfo::GetRenderCPUTime()
{
    return Instance()._renderTime;
}

DebugInfo::DebugInfo(rhi::Device* device)
    : _fps(0)
    , _msPerFrame(0)
    , _totalRenderFrames(0)
    , _totalRenderTime(0.0)
    , _totalUpdateFrames(0)
    , _totalUpdateTime(0.0)
{
    _statisticsQuery = device->CreateStatisticsQuery("Debug Info");
}

DebugInfo::~DebugInfo()
{
}

DebugInfo& DebugInfo::Instance()
{
    ASSERT(_instance, "DebugInfo instance not created");
    return *_instance;
}
