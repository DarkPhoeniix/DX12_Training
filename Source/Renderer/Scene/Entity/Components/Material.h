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
			, NormalMapTextureHandle(InvalidTextureHandle)
			, MetalnessTextureHandle(InvalidTextureHandle)
			, RoughnessTextureHandle(InvalidTextureHandle)
        {
        }

        TextureHandle AlbedoTextureHandle;
        TextureHandle NormalMapTextureHandle;
        TextureHandle MetalnessTextureHandle;
        TextureHandle RoughnessTextureHandle;
    };
} // namespace scene
