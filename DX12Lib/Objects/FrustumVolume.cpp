#include "stdafx.h"
#include "FrustumVolume.h"

#include "AABBVolume.h"

#include <DirectXCollision.h>

using namespace DirectX;

namespace
{
    Plane buildPlane(XMVECTOR vec)
    {
        float length = XMVectorGetX(XMVector3Length(vec));

        Plane p;
        p.normal = XMVector3Normalize(vec);
        p.distance = XMVectorGetW(vec);

        return p;
    }

    bool intersectWithPlane(const Plane& plane, XMMATRIX transform, const AABBVolume& aabb)
    {
        //XMVECTOR planeNorm = XMVector4Transform(plane.normal, transform);
        XMVECTOR planeNorm = plane.normal;
        XMVECTOR min = XMVectorZero();
        XMVECTOR max = XMVectorZero();

        // X axis 
        if (XMVectorGetX(planeNorm) > 0.0f)
        {
            min = XMVectorSetX(min, XMVectorGetX(aabb.min));
            max = XMVectorSetX(max, XMVectorGetX(aabb.max));
        }
        else
        {
            min = XMVectorSetX(min, XMVectorGetX(aabb.max));
            max = XMVectorSetX(max, XMVectorGetX(aabb.min));
        }
        // Y axis 
        if (XMVectorGetY(planeNorm) > 0.0f)
        {
            min = XMVectorSetY(min, XMVectorGetY(aabb.min));
            max = XMVectorSetY(max, XMVectorGetY(aabb.max));
        }
        else
        {
            min = XMVectorSetY(min, XMVectorGetY(aabb.max));
            max = XMVectorSetY(max, XMVectorGetY(aabb.min));
        }
        // Z axis 
        if (XMVectorGetZ(planeNorm) > 0.0f)
        {
            min = XMVectorSetZ(min, XMVectorGetZ(aabb.min));
            max = XMVectorSetZ(max, XMVectorGetZ(aabb.max));
        }
        else
        {
            min = XMVectorSetZ(min, XMVectorGetZ(aabb.max));
            max = XMVectorSetZ(max, XMVectorGetZ(aabb.min));
        }

        if ((XMVectorGetX(XMVector4Dot(planeNorm, min)) + plane.distance < 0.0f) &&
            (XMVectorGetX(XMVector4Dot(planeNorm, max)) + plane.distance <= 0.0f))
        {
            return false;
        }

        return true;


    }
}

void FrustumVolume::buildFromProjMatrix(const DirectX::XMMATRIX projectionMatrix)
{
    XMMATRIX viewProj = projectionMatrix;
    //leftPlane = buildPlane(XMVectorAdd(viewProj.r[3], viewProj.r[0]));
    //rightPlane = buildPlane(XMVectorSubtract(viewProj.r[3], viewProj.r[0]));
    //bottomPlane = buildPlane(XMVectorAdd(viewProj.r[3], viewProj.r[1]));
    //topPlane = buildPlane(XMVectorSubtract(viewProj.r[3], viewProj.r[1]));
    //nearPlane = buildPlane(XMVectorAdd(viewProj.r[3], viewProj.r[2]));
    //farPlane = buildPlane(XMVectorSubtract(viewProj.r[3], viewProj.r[2]));

    // Calculate near plane of frustum.
    nearPlane.normal.m128_f32[0]  = viewProj.r[0].m128_f32[3] + viewProj.r[0].m128_f32[2];
    nearPlane.normal.m128_f32[1]  = viewProj.r[1].m128_f32[3] + viewProj.r[1].m128_f32[2];
    nearPlane.normal.m128_f32[2]  = viewProj.r[2].m128_f32[3] + viewProj.r[2].m128_f32[2];
    nearPlane.distance            = viewProj.r[3].m128_f32[3] + viewProj.r[2].m128_f32[2];
    nearPlane.normal = XMVector3Normalize(nearPlane.normal);

    // Calculate far plane of frustum.
    farPlane.normal.m128_f32[0]   = viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[2];
    farPlane.normal.m128_f32[1]   = viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[2];
    farPlane.normal.m128_f32[2]   = viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[2];
    farPlane.distance             = viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[2];
    farPlane.normal = XMVector3Normalize(farPlane.normal);

    // Calculate left plane of frustum.
    leftPlane.normal.m128_f32[0]  = viewProj.r[0].m128_f32[3] + viewProj.r[0].m128_f32[0];
    leftPlane.normal.m128_f32[1]  = viewProj.r[1].m128_f32[3] + viewProj.r[1].m128_f32[0];
    leftPlane.normal.m128_f32[2]  = viewProj.r[2].m128_f32[3] + viewProj.r[2].m128_f32[0];
    leftPlane.distance            = viewProj.r[3].m128_f32[3] + viewProj.r[3].m128_f32[0];
    leftPlane.normal = XMVector3Normalize(leftPlane.normal);

    // Calculate right plane of frustum.
    rightPlane.normal.m128_f32[0] = viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[0];
    rightPlane.normal.m128_f32[1] = viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[0];
    rightPlane.normal.m128_f32[2] = viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[0];
    rightPlane.distance           = viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[0];
    rightPlane.normal = XMVector3Normalize(rightPlane.normal);

    // Calculate top plane of frustum.
    topPlane.normal.m128_f32[0]   = viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[1];
    topPlane.normal.m128_f32[1]   = viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[1];
    topPlane.normal.m128_f32[2]   = viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[1];
    topPlane.distance             = viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[1];
    topPlane.normal = XMVector3Normalize(topPlane.normal);

    // Calculate bottom plane of frustum.
    bottomPlane.normal.m128_f32[0] = viewProj.r[0].m128_f32[3] + viewProj.r[0].m128_f32[1];
    bottomPlane.normal.m128_f32[1] = viewProj.r[1].m128_f32[3] + viewProj.r[1].m128_f32[1];
    bottomPlane.normal.m128_f32[2] = viewProj.r[2].m128_f32[3] + viewProj.r[2].m128_f32[1];
    bottomPlane.distance           = viewProj.r[3].m128_f32[3] + viewProj.r[3].m128_f32[1];
    bottomPlane.normal = XMVector3Normalize(bottomPlane.normal);
}

bool intersect(const FrustumVolume& frustum, const AABBVolume& aabb)
{
    if (!intersectWithPlane(frustum.bottomPlane, frustum.transform, aabb)) return false;
    if (!intersectWithPlane(frustum.topPlane, frustum.transform, aabb)) return false;
    if (!intersectWithPlane(frustum.leftPlane, frustum.transform, aabb)) return false;
    if (!intersectWithPlane(frustum.rightPlane, frustum.transform, aabb)) return false;
    if (!intersectWithPlane(frustum.nearPlane, frustum.transform, aabb)) return false;
    if (!intersectWithPlane(frustum.farPlane, frustum.transform, aabb)) return false;

    return true;
}
