#pragma once

#include "Scene/Volumes/IVolume.h"

namespace SceneLayer
{
    class AABBVolume;

    class OBBVolume : public IVolume
    {
    public:
        DirectX::XMMATRIX Bounds = DirectX::XMMatrixIdentity();
    };

    AABBVolume CombineOBBs(const std::vector<OBBVolume>& volumes);
} // namespace SceneLayer
