#pragma once

#include "Texture.h"
#include "Scene/Entity/Components/IComponent.h"

namespace SceneLayer
{
    struct Material : public IComponent
    {
        Material()
            : IComponent("Material")
        {
        }

        std::shared_ptr<dx12::Texture> Albedo;
        std::shared_ptr<dx12::Texture> NormalMap;
        std::shared_ptr<dx12::Texture> Metalness;
        std::shared_ptr<dx12::Texture> Roughness;
    };
} // namespace SceneLayer
