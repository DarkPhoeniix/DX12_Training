#pragma once

#include "Scene/Entity/Components/IComponent.h"

namespace SceneLayer
{
    struct Transformation : public IComponent
    {
        Transformation()
            : IComponent("Transformation")
            , Transform(DirectX::XMMatrixIdentity())
        {
        }

        DirectX::XMMATRIX Transform;
    };
} // namespace SceneLayer
