#pragma once

#include "DXObjects/Texture.h"

namespace SceneLayer
{
    class Scene;
} // namespace SceneLayer

namespace Core
{
    class GBuffer
    {
    public:
        void Init(const DirectX::XMUINT2& size);

        Core::Texture& GetPositionTexture();
        const Core::Texture& GetPositionTexture() const;

        Core::Texture& GetNormalTexture();
        const Core::Texture& GetNormalTexture() const;

        Core::Texture& GetAlbedoMetalnessTexture();
        const Core::Texture& GetAlbedoMetalnessTexture() const;

    private:
        Core::Texture _position;
        Core::Texture _normalSpecular;
        Core::Texture _albedoMetalness;
    };
} // namespace Core
