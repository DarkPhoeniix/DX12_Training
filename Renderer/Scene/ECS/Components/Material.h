#pragma once

#include "Texture.h"
#include "Scene/ECS/Components.h"

struct COMPONENT Material : public IComponent
{
    Material()
        : IComponent("Material")
    {   }

    std::shared_ptr<dx12::Texture> Albedo;
    std::shared_ptr<dx12::Texture> NormalMap;
    std::shared_ptr<dx12::Texture> Metalness;
    std::shared_ptr<dx12::Texture> Roughness;
};
