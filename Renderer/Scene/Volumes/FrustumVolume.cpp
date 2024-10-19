#include "stdafx.h"

#include "FrustumVolume.h"

#include "Scene/Volumes/AABBVolume.h"

using namespace DirectX;

namespace
{
    bool IntersectWithPlane(const XMVECTOR& plane, const SceneLayer::AABBVolume& aabb)
    {
        bool result = true;

        XMVECTOR aabbCenter = (aabb.max + aabb.min) * 0.5f;
        XMVECTOR aabbHalfSize = (aabb.max - aabb.min) * 0.5f;

        float rg = abs(XMVectorGetX(plane) * XMVectorGetX(aabbHalfSize))
                 + abs(XMVectorGetY(plane) * XMVectorGetY(aabbHalfSize))
                 + abs(XMVectorGetZ(plane) * XMVectorGetZ(aabbHalfSize));

        if (XMVectorGetX(XMPlaneDotCoord(plane, aabbCenter)) <= -rg)
        {
            result = false;
        }

        return result;
    }
}

namespace SceneLayer
{
    void FrustumVolume::BuildFromProjMatrix(const DirectX::XMMATRIX projectionMatrix)
    {
        // Calculate left plane of frustum.
        planes[0] = XMVectorSet(projectionMatrix.r[0].m128_f32[3] + projectionMatrix.r[0].m128_f32[0],
            projectionMatrix.r[1].m128_f32[3] + projectionMatrix.r[1].m128_f32[0],
            projectionMatrix.r[2].m128_f32[3] + projectionMatrix.r[2].m128_f32[0],
            projectionMatrix.r[3].m128_f32[3] + projectionMatrix.r[3].m128_f32[0]);
        planes[0] = XMPlaneNormalize(planes[0]);

        // Calculate right plane of frustum.
        planes[1] = XMVectorSet(projectionMatrix.r[0].m128_f32[3] - projectionMatrix.r[0].m128_f32[0],
            projectionMatrix.r[1].m128_f32[3] - projectionMatrix.r[1].m128_f32[0],
            projectionMatrix.r[2].m128_f32[3] - projectionMatrix.r[2].m128_f32[0],
            projectionMatrix.r[3].m128_f32[3] - projectionMatrix.r[3].m128_f32[0]);
        planes[1] = XMPlaneNormalize(planes[1]);

        // Calculate bottom plane of frustum.
        planes[2] = XMVectorSet(projectionMatrix.r[0].m128_f32[3] + projectionMatrix.r[0].m128_f32[1],
            projectionMatrix.r[1].m128_f32[3] + projectionMatrix.r[1].m128_f32[1],
            projectionMatrix.r[2].m128_f32[3] + projectionMatrix.r[2].m128_f32[1],
            projectionMatrix.r[3].m128_f32[3] + projectionMatrix.r[3].m128_f32[1]);
        planes[2] = XMPlaneNormalize(planes[2]);

        // Calculate top plane of frustum.
        planes[3] = XMVectorSet(projectionMatrix.r[0].m128_f32[3] - projectionMatrix.r[0].m128_f32[1],
            projectionMatrix.r[1].m128_f32[3] - projectionMatrix.r[1].m128_f32[1],
            projectionMatrix.r[2].m128_f32[3] - projectionMatrix.r[2].m128_f32[1],
            projectionMatrix.r[3].m128_f32[3] - projectionMatrix.r[3].m128_f32[1]);
        planes[3] = XMPlaneNormalize(planes[3]);

        // Calculate near plane of frustum.
        planes[4] = XMVectorSet(projectionMatrix.r[0].m128_f32[3] + projectionMatrix.r[0].m128_f32[2],
            projectionMatrix.r[1].m128_f32[3] + projectionMatrix.r[1].m128_f32[2],
            projectionMatrix.r[2].m128_f32[3] + projectionMatrix.r[2].m128_f32[2],
            projectionMatrix.r[3].m128_f32[3] + projectionMatrix.r[3].m128_f32[2]);
        planes[4] = XMPlaneNormalize(planes[4]);

        // Calculate far plane of frustum.
        planes[5] = XMVectorSet(projectionMatrix.r[0].m128_f32[3] - projectionMatrix.r[0].m128_f32[2],
            projectionMatrix.r[1].m128_f32[3] - projectionMatrix.r[1].m128_f32[2],
            projectionMatrix.r[2].m128_f32[3] - projectionMatrix.r[2].m128_f32[2],
            projectionMatrix.r[3].m128_f32[3] - projectionMatrix.r[3].m128_f32[2]);
        planes[5] = XMPlaneNormalize(planes[5]);
    }

    bool Intersect(const FrustumVolume& frustum, const AABBVolume& aabb, const DirectX::XMMATRIX& globalTransform)
    {
        AABBVolume transformedAABB = aabb;
        transformedAABB.min = DirectX::XMVector4Transform(aabb.min, globalTransform);
        transformedAABB.max = DirectX::XMVector4Transform(aabb.max, globalTransform);

        for (const XMVECTOR& plane : frustum.planes)
        {
            if (!IntersectWithPlane(plane, transformedAABB))
            {
                return false;
            }
        }

        return true;
    }
} // namespace SceneLayer
