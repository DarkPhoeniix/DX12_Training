#pragma once

#include "Scene/Volumes/IVolume.h"

namespace scene
{
    class AABBVolume;

    class FrustumVolume : public IVolume
    {
    public:
        void BuildFromProjMatrix(const DirectX::XMMATRIX projectionMatrix);

        friend bool Intersect(const FrustumVolume& frustum, const AABBVolume& aabb, const DirectX::XMMATRIX& globalTransform);

        DirectX::XMVECTOR planes[6];
    };
} // namespace scene
