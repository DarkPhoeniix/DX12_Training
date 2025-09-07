#pragma once

class RenderSettings
{
public:
    struct BloomParameters
    {
        float Intensity = 0.04f;
        float Radius = 1.0f;
    };

    struct ToneMappingParameters
    {
        float MiddleGrey = 0.18f;
        float WhitePoint = 3.5f;
        float Gamma = 2.2f;

        float MinLogLuminance = -10.0f;
        float MaxLogLuminance = 4.0f;
    };

    RenderSettings(const RenderSettings&) = delete;
    RenderSettings& operator+(const RenderSettings&) = delete;

    static bool& UseIBL();
    static bool& RenderDebugVolumes();
    static bool& RenderDebugArmature();
    static bool& UseSSAO();
    static bool& UseFXAA();
    static bool& DebugFXAA();

    static bool& UseBloom();
    static BloomParameters& Bloom();
    static ToneMappingParameters& ToneMapping();

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

    bool _useBloom;
    BloomParameters _bloom;
    ToneMappingParameters _toneMapping;
};
