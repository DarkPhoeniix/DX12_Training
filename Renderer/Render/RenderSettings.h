#pragma once

class RenderSettings
{
public:
    RenderSettings(const RenderSettings&) = delete;
    RenderSettings& operator+(const RenderSettings&) = delete;

    static bool& RenderDebugVolumes();
    static bool& RenderDebugArmature();
    static bool& UseFXAA();

private:
    RenderSettings();
    ~RenderSettings() = default;

    static RenderSettings& Instance();

    bool _renderDebugVolumes;
    bool _renderDebugArmature;
    bool _useFXAA;
};
