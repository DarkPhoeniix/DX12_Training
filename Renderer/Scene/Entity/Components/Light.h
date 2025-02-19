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

    class Light : public IComponent
    {
    public:
        Light()
            : IComponent("Light")
        {
        }

        LightType Type;

        DirectX::XMVECTOR Direction;
        DirectX::XMVECTOR Color;

        float Intensity;
        float Range;

        float OuterAngle;
        float InnerAngle;

        bool CastShadows;
        std::shared_ptr<dx12::Resource> ShadowMap;
    };
}
