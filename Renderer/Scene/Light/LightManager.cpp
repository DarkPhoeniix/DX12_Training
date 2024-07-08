#include "stdafx.h"

#include "LightManager.h"

#include "DXObjects/GraphicsCommandList.h"

#include "Scene/Light/DirectionalLight.h"
#include "Scene/Light/PointLight.h"

void LightManager::Init()
{
    Core::EResourceType CBVType = Core::EResourceType::Dynamic | Core::EResourceType::Buffer | Core::EResourceType::StrideAlignment;
    Core::EResourceType SRVType = Core::EResourceType::Dynamic | Core::EResourceType::Buffer;

    {
        Core::ResourceDescription desc;
        desc.SetResourceType(CBVType);
        desc.SetSize({ sizeof(LightDesc), 1 });
        desc.SetStride(1);
        desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

        _lightDescResource.SetResourceDescription(desc);
        _lightDescResource.CreateCommitedResource();
        _lightDescResource.SetName("Lights description");
    }

    {
        Core::ResourceDescription desc;
        desc.SetResourceType(SRVType);
        desc.SetSize({ sizeof(DirectionalLight), _directionalLights.size() });
        desc.SetStride(1);
        desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

        _directionalLightsResource.SetResourceDescription(desc);
        _directionalLightsResource.CreateCommitedResource();
        _directionalLightsResource.SetName("Directional lights buffer");
    }

    {
        Core::ResourceDescription desc;
        desc.SetResourceType(SRVType);
        desc.SetSize({ sizeof(PointLight), _pointLights.size() });
        desc.SetStride(1);
        desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

        _pointLightsResource.SetResourceDescription(desc);
        _pointLightsResource.CreateCommitedResource();
        _pointLightsResource.SetName("Point lights buffer");
    }
}

void LightManager::SetupLights(Core::GraphicsCommandList& commandList)
{
    commandList.SetCBV(1, _lightDescResource.OffsetGPU(0));

    commandList.SetSRV(3, _directionalLightsResource.OffsetGPU(0));
    commandList.SetSRV(4, _pointLightsResource.OffsetGPU(0));
}

void LightManager::AddDirectionalLight(const DirectionalLight& light)
{
    _directionalLights.push_back(light);
    _lightDesc.directionalLightsNum++;
}

void LightManager::AddPointLight(const PointLight& light)
{
    _pointLights.push_back(light);
    _lightDesc.pointLightsNum++;
}
