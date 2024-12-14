#pragma once

#include "Scene/ECS/Components.h"

enum class LightType
{
    Directional,
    Point,
    Spot
};

struct COMPONENT Light : public IComponent
{
    Light()
        : IComponent("Light")
    {   }

    LightType Type;

    DirectX::XMVECTOR Direction;
    DirectX::XMVECTOR Color;

    float Intensity;
    float Range;
};
