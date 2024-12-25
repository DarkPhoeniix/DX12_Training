#pragma once

#include "Scene/Volumes/IVolume.h"

namespace SceneLayer
{
    class OBBVolume : public IVolume
    {
    public:
        DirectX::XMMATRIX Bounds = DirectX::XMMatrixIdentity();
    };
} // namespace SceneLayer
