#pragma once

#include "Scene/Entity/Components/IComponent.h"

namespace scene
{
    enum class LightType
    {
        Directional,
        Point,
        Spot
    };

    struct Light : public IComponent
    {
        Light()
            : IComponent("Light")
        {
        }

        LightType Type;

        DirectX::XMVECTOR Direction;
        DirectX::XMVECTOR Color;

        float Intensity;
        float Range;
    };
}
