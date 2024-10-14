#pragma once

#include "Scene/Volumes/IVolume.h"

namespace SceneLayer
{
    class AABBVolume : public IVolume
    {
    public:
        DirectX::XMVECTOR min;
        DirectX::XMVECTOR max;
    };
} // namespace SceneLayer
