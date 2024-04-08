#include "stdafx.h"
#include "FrustumVolume.h"

#include "AABBVolume.h"

#include <DirectXCollision.h>

using namespace DirectX;

namespace
{
    bool intersectWithPlane(XMVECTOR plane, AABBVolume aabb)
    {
        bool result = true;

        XMVECTOR aabbCenter = (aabb.max + aabb.min) / 2.0f;
        XMVECTOR aabbHalfSize = (aabb.max - aabb.min) / 2.0f;

        float rg = abs(XMVectorGetX(plane) * XMVectorGetX(aabbHalfSize))
                 + abs(XMVectorGetY(plane) * XMVectorGetY(aabbHalfSize))
                 + abs(XMVectorGetZ(plane) * XMVectorGetZ(aabbHalfSize));

        if (XMVectorGetX(XMPlaneDotCoord(plane, aabbCenter)) <= -rg)
            result = false;

        return result;
    }
}

void FrustumVolume::buildFromProjMatrix(const DirectX::XMMATRIX projectionMatrix)
{
    // Calculate left plane of frustum.
    leftPlane = XMVectorSet(projectionMatrix.r[0].m128_f32[3] + projectionMatrix.r[0].m128_f32[0],
                            projectionMatrix.r[1].m128_f32[3] + projectionMatrix.r[1].m128_f32[0],
                            projectionMatrix.r[2].m128_f32[3] + projectionMatrix.r[2].m128_f32[0],
                            projectionMatrix.r[3].m128_f32[3] + projectionMatrix.r[3].m128_f32[0]);
    leftPlane = XMPlaneNormalize(leftPlane);

    // Calculate right plane of frustum.
    rightPlane = XMVectorSet(projectionMatrix.r[0].m128_f32[3] - projectionMatrix.r[0].m128_f32[0],
                             projectionMatrix.r[1].m128_f32[3] - projectionMatrix.r[1].m128_f32[0],
                             projectionMatrix.r[2].m128_f32[3] - projectionMatrix.r[2].m128_f32[0],
                             projectionMatrix.r[3].m128_f32[3] - projectionMatrix.r[3].m128_f32[0]);
    rightPlane = XMPlaneNormalize(rightPlane);

    // Calculate bottom plane of frustum.
    bottomPlane = XMVectorSet(projectionMatrix.r[0].m128_f32[3] + projectionMatrix.r[0].m128_f32[1],
                              projectionMatrix.r[1].m128_f32[3] + projectionMatrix.r[1].m128_f32[1],
                              projectionMatrix.r[2].m128_f32[3] + projectionMatrix.r[2].m128_f32[1],
                              projectionMatrix.r[3].m128_f32[3] + projectionMatrix.r[3].m128_f32[1]);
    bottomPlane = XMPlaneNormalize(bottomPlane);

    // Calculate top plane of frustum.
    topPlane = XMVectorSet(projectionMatrix.r[0].m128_f32[3] - projectionMatrix.r[0].m128_f32[1],
                           projectionMatrix.r[1].m128_f32[3] - projectionMatrix.r[1].m128_f32[1],
                           projectionMatrix.r[2].m128_f32[3] - projectionMatrix.r[2].m128_f32[1],
                           projectionMatrix.r[3].m128_f32[3] - projectionMatrix.r[3].m128_f32[1]);
    topPlane = XMPlaneNormalize(topPlane);

    // Calculate near plane of frustum.
    nearPlane = XMVectorSet(projectionMatrix.r[0].m128_f32[3] + projectionMatrix.r[0].m128_f32[2],
                            projectionMatrix.r[1].m128_f32[3] + projectionMatrix.r[1].m128_f32[2],
                            projectionMatrix.r[2].m128_f32[3] + projectionMatrix.r[2].m128_f32[2],
                            projectionMatrix.r[3].m128_f32[3] + projectionMatrix.r[3].m128_f32[2]);
    nearPlane = XMPlaneNormalize(nearPlane);

    // Calculate far plane of frustum.
    farPlane = XMVectorSet(projectionMatrix.r[0].m128_f32[3] - projectionMatrix.r[0].m128_f32[2],
                           projectionMatrix.r[1].m128_f32[3] - projectionMatrix.r[1].m128_f32[2],
                           projectionMatrix.r[2].m128_f32[3] - projectionMatrix.r[2].m128_f32[2],
                           projectionMatrix.r[3].m128_f32[3] - projectionMatrix.r[3].m128_f32[2]);
    farPlane = XMPlaneNormalize(farPlane);
}

bool intersect(const FrustumVolume& frustum, const AABBVolume& aabb)
{
    if (!intersectWithPlane(frustum.bottomPlane, aabb)) return false;
    if (!intersectWithPlane(frustum.topPlane, aabb)) return false;
    if (!intersectWithPlane(frustum.leftPlane, aabb)) return false;
    if (!intersectWithPlane(frustum.rightPlane, aabb)) return false;
    if (!intersectWithPlane(frustum.nearPlane, aabb)) return false;
    if (!intersectWithPlane(frustum.farPlane, aabb)) return false;

    return true;
}
