#pragma once

#include "Texture.h"
#include "Scene/ECS/Components.h"

struct COMPONENT Skybox : public IComponent
{
    Skybox()
        : IComponent("Skybox")
    {   }

    std::shared_ptr<dx12::Texture> SkydomeTexture;
};
