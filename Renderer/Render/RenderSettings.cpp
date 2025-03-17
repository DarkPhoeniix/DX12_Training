#include "RendererPCH.h"

#include "RenderSettings.h"

bool& RenderSettings::RenderDebugVolumes()
{
    return Instance()._renderDebugVolumes;
}

bool& RenderSettings::RenderDebugArmature()
{
    return Instance()._renderDebugArmature;
}

bool& RenderSettings::UseFXAA()
{
    return Instance()._useFXAA;
}

RenderSettings::RenderSettings()
    : _renderDebugVolumes(false)
    , _renderDebugArmature(false)
    , _useFXAA(false)
{
}

RenderSettings& RenderSettings::Instance()
{
    static RenderSettings instance;
    return instance;
}
