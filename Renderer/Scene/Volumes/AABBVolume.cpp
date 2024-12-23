#include "stdafx.h"

#include "AABBVolume.h"

namespace SceneLayer
{
    AABBVolume CombineAABBs(const std::vector<AABBVolume>& volumes)
    {
        AABBVolume result;

        for (const auto& aabb : volumes)
        {
            result.Min = DirectX::XMVectorMin(result.Min, aabb.Min);
            result.Min = DirectX::XMVectorMin(result.Min, aabb.Max);
            result.Max = DirectX::XMVectorMax(result.Max, aabb.Max);
            result.Max = DirectX::XMVectorMax(result.Max, aabb.Min);
        }

        return result;
    }
} // namespace SceneLayer
