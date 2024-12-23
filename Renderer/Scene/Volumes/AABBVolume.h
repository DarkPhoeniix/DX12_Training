#pragma once

#include "Scene/Volumes/IVolume.h"

namespace SceneLayer
{
    class AABBVolume : public IVolume
    {
    public:
        AABBVolume()
            : Min(DirectX::XMVectorSet(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 1.0f))
            , Max(DirectX::XMVectorSet(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), 1.0f))
        {   }

        AABBVolume(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max)
            : Min(min)
            , Max(max)
        {   }

        DirectX::XMVECTOR Min;
        DirectX::XMVECTOR Max;
    };

    AABBVolume CombineAABBs(const std::vector<AABBVolume>& volumes);
} // namespace SceneLayer
