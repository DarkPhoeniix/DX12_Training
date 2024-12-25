#pragma once

#include "Scene/Volumes/IVolume.h"

namespace SceneLayer
{
    class CapsuleVolume : public IVolume
    {
    public:
        DirectX::XMVECTOR Points[2];
        float Radius;
    };
} // namespace SceneLayer
