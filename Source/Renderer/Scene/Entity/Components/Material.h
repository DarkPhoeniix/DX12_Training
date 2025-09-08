#pragma once

#include "Scene/Entity/Components/IComponent.h"
#include "Core/TextureManager.h"

namespace scene
{
    class Material : public IComponent
    {
    public:
        Material()
            : IComponent("Material")
            , AlbedoTextureHandle(InvalidTextureHandle)
            , AlbedoColor(DirectX::XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f))
            , NormalMapTextureHandle(InvalidTextureHandle)
            , NormalVector(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f))
            , MetalnessTextureHandle(InvalidTextureHandle)
            , MetallicValue(0.0f)
            , RoughnessTextureHandle(InvalidTextureHandle)
            , RoughnessValue(1.0f)
            , EmissionTextureHandle(InvalidTextureHandle)
            , EmissionColor(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f))
            , EmissionIntensity(0.0f)
        {
        }

        TextureHandle AlbedoTextureHandle;
        DirectX::XMVECTOR AlbedoColor;

        TextureHandle NormalMapTextureHandle;
        DirectX::XMVECTOR NormalVector;

        TextureHandle MetalnessTextureHandle;
        float MetallicValue;

        TextureHandle RoughnessTextureHandle;
        float RoughnessValue;

        TextureHandle EmissionTextureHandle;
        DirectX::XMVECTOR EmissionColor;
        float EmissionIntensity;
    };
} // namespace scene
