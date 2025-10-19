#pragma once

class RenderSettings
{
public:
    struct BloomParameters
    {
        float Intensity = 0.2f;
        float Intensity1 = 0.4f;
        float Radius = 2.0f;
    };

    struct ToneMappingParameters
    {
        float MiddleGrey = 0.18f;
        float WhitePoint = 3.5f;
        float Gamma = 2.2f;

        float MinLogLuminance = -10.0f;
        float MaxLogLuminance = 4.0f;
    };

    struct SSAOParameters
    {
        bool Enabled = true;

        float Radius = 2.5f;
        float Bias = 0.05f;

        int BlurRadius = 5;
        float DepthThreshold = 0.2f;
        float Sharpness = 50.0f;
    };

    struct DebugViewParameters
    {
        bool ShowAlbedo = false;
        bool ShowNormals = false;
        bool ShowMetalness = false;
        bool ShowRoughness = false;
        bool ShowSSAO = false;
        bool ShowEmission = false;

        void DisableAll()
        {
            ShowAlbedo = false;
            ShowNormals = false;
            ShowMetalness = false;
            ShowRoughness = false;
            ShowSSAO = false;
            ShowEmission = false;
        }
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
    static SSAOParameters& SSAO();
    static DebugViewParameters& DebugView();

private:
    RenderSettings();
    ~RenderSettings() = default;

    static RenderSettings& Instance();

    bool _useIBL;
    bool _renderDebugVolumes;
    bool _renderDebugArmature;
    bool _useFXAA;
    bool _debugFXAA;

    bool _useBloom;
    BloomParameters _bloom;
    ToneMappingParameters _toneMapping;
    SSAOParameters _ssao;
    DebugViewParameters _debugView;
};
