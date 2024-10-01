#pragma once

#include "Scene/Volumes/IVolume.h"

namespace SceneLayer
{
    class AABBVolume;

    class FrustumVolume : public IVolume
    {
    public:
        void BuildFromProjMatrix(const DirectX::XMMATRIX projectionMatrix);

        friend bool Intersect(const FrustumVolume& frustum, const AABBVolume& aabb);

        DirectX::XMVECTOR planes[6];
    };
} // namespace SceneLayer
