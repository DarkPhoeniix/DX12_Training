#pragma once

class RenderSettings
{
public:
    RenderSettings(const RenderSettings&) = delete;
    RenderSettings& operator+(const RenderSettings&) = delete;

    static bool& UseIBL();
    static bool& RenderDebugVolumes();
    static bool& RenderDebugArmature();
    static bool& UseSSAO();
    static bool& UseFXAA();
    static bool& DebugFXAA();

private:
    RenderSettings();
    ~RenderSettings() = default;

    static RenderSettings& Instance();

    bool _useIBL;
    bool _renderDebugVolumes;
    bool _renderDebugArmature;
    bool _useSSAO;
    bool _useFXAA;
    bool _debugFXAA;
};
