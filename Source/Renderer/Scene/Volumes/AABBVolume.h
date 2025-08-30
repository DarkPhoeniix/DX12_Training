#pragma once

#include "Scene/Volumes/IVolume.h"

namespace scene
{
    class AABBVolume : public IVolume
    {
    public:
        AABBVolume()
            : Min(DirectX::XMVectorSet(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 1.0f))
            , Max(DirectX::XMVectorSet(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), 1.0f))
        {
        }

        AABBVolume(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max)
            : Min(min)
            , Max(max)
        {
        }

        AABBVolume Transform(const DirectX::XMMATRIX& transform);

        DirectX::XMVECTOR Min;
        DirectX::XMVECTOR Max;
    };
} // namespace scene
