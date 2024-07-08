#pragma once

namespace Core
{
    class GraphicsCommandList;
}

class DirectionalLight;
class PointLight;

class LightManager
{
public:
    LightManager() = default;
    ~LightManager() = default;

    void Init();

    void SetupLights(Core::GraphicsCommandList& commandList);

    void AddDirectionalLight(const DirectionalLight& light);
    void AddPointLight(const PointLight& light);
    
private:
    struct LightDesc
    {
        UINT directionalLightsNum;
        UINT pointLightsNum;
    } _lightDesc;
    Core::Resource _lightDescResource;

    std::vector<DirectionalLight> _directionalLights;
    Core::Resource _directionalLightsResource;

    std::vector<PointLight> _pointLights;
    Core::Resource _pointLightsResource;
};
