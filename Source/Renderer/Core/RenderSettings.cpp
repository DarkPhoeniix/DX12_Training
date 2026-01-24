#include "RendererPCH.h"

#include "RenderSettings.h"

bool& RenderSettings::EnableCPUFrustumCulling()
{
    return Instance()._enableCPUFrustumCulling;
}

bool& RenderSettings::UseIBL()
{
    return Instance()._useIBL;
}

bool& RenderSettings::RenderDebugVolumes()
{
    return Instance()._renderDebugVolumes;
}

bool& RenderSettings::RenderDebugArmature()
{
    return Instance()._renderDebugArmature;
}

bool& RenderSettings::UseSSAO()
{
    return Instance().SSAO().Enabled;
}

bool& RenderSettings::UseFXAA()
{
    return Instance()._useFXAA;
}

bool& RenderSettings::DebugFXAA()
{
    return Instance()._debugFXAA;
}

bool& RenderSettings::UseBloom()
{
    return Instance()._useBloom;
}

RenderSettings::BloomParameters& RenderSettings::Bloom()
{
    return Instance()._bloom;
}

RenderSettings::ToneMappingParameters& RenderSettings::ToneMapping()
{
    return Instance()._toneMapping;
}

RenderSettings::SSAOParameters& RenderSettings::SSAO()
{
    return Instance()._ssao;
}

RenderSettings::DebugViewParameters& RenderSettings::DebugView()
{
    return Instance()._debugView;
}

RenderSettings::RenderSettings()
    : _enableCPUFrustumCulling(true)
    , _useIBL(true)
    , _renderDebugVolumes(false)
    , _renderDebugArmature(false)
    , _useFXAA(true)
    , _debugFXAA(false)
    , _useBloom(true)
{
}

RenderSettings& RenderSettings::Instance()
{
    static RenderSettings instance;
    return instance;
}
