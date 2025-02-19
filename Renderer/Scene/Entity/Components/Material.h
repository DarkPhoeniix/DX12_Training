#pragma once

#include "Texture.h"
#include "Scene/Entity/Components/IComponent.h"

namespace scene
{
    class Material : public IComponent
    {
    public:
        Material()
            : IComponent("Material")
        {
        }

        std::shared_ptr<dx12::Texture> Albedo;
        std::shared_ptr<dx12::Texture> NormalMap;
        std::shared_ptr<dx12::Texture> Metalness;
        std::shared_ptr<dx12::Texture> Roughness;
    };
} // namespace scene
