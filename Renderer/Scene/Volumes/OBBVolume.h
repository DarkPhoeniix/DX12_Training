#pragma once

#include "Scene/Volumes/IVolume.h"

namespace scene
{
    class AABBVolume;

    class OBBVolume : public IVolume
    {
    public:
        DirectX::XMMATRIX Bounds = DirectX::XMMatrixIdentity();
    };

    AABBVolume CombineOBBs(const std::vector<OBBVolume>& volumes);
} // namespace scene
