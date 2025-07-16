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

        std::string Albedo;
        std::string NormalMap;
        std::string Metalness;
        std::string Roughness;
    };
} // namespace scene
