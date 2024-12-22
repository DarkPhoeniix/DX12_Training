#pragma once

#include "Scene/ECS/Components.h"

struct COMPONENT Transformation : public IComponent
{
    Transformation()
        : IComponent("Transformation")
        , Transform(DirectX::XMMatrixIdentity())
    {   }

    DirectX::XMMATRIX Transform;
};
