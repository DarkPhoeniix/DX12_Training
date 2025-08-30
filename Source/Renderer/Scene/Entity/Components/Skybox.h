#pragma once

#include "Scene/Entity/Components/IComponent.h"
#include "Core/TextureManager.h"

namespace scene
{
    class Skybox : public IComponent
    {
    public:
        Skybox()
            : IComponent("Skybox")
            , SkydomeTextureHandle(InvalidTextureHandle)
        {
        }

        TextureHandle SkydomeTextureHandle;
    };
} // namespace scene
