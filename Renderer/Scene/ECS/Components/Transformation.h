#pragma once

#include "Scene/ECS/Components.h"

struct COMPONENT Transformation : public IComponent
{
    Transformation()
        : IComponent("Transformation")
    {   }

    DirectX::XMMATRIX Transform;
};
