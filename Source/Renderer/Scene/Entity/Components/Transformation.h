#pragma once

#include "Scene/Entity/Components/IComponent.h"

namespace scene
{
    class Transformation : public IComponent
    {
    public:
        Transformation()
            : IComponent("Transformation")
            , Transform(DirectX::XMMatrixIdentity())
        {
        }

        DirectX::XMMATRIX Transform;
    };
} // namespace scene
