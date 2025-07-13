#include "RendererPCH.h"

#include "AABBVolume.h"

using namespace DirectX;

namespace scene
{
    AABBVolume AABBVolume::Transform(const XMMATRIX& transform)
    {
        XMVECTOR corners[8];

        XMFLOAT3 min, max;
        XMStoreFloat3(&min, Min);
        XMStoreFloat3(&max, Max);

        corners[0] = XMVectorSet(min.x, min.y, min.z, 1.0f);
        corners[1] = XMVectorSet(min.x, min.y, max.z, 1.0f);
        corners[2] = XMVectorSet(min.x, max.y, min.z, 1.0f);
        corners[3] = XMVectorSet(min.x, max.y, max.z, 1.0f);
        corners[4] = XMVectorSet(max.x, min.y, min.z, 1.0f);
        corners[5] = XMVectorSet(max.x, min.y, max.z, 1.0f);
        corners[6] = XMVectorSet(max.x, max.y, min.z, 1.0f);
        corners[7] = XMVectorSet(max.x, max.y, max.z, 1.0f);

        XMVECTOR boxMin = XMVector4Transform(corners[0], transform);
        XMVECTOR boxMax = boxMin;

        XMVECTOR point;
        for (size_t cornerIndex = 1; cornerIndex < 8; ++cornerIndex)
        {
            point = XMVector3TransformCoord(corners[cornerIndex], transform);

            boxMin = XMVectorMin(boxMin, point);
            boxMax = XMVectorMax(boxMax, point);
        }

        AABBVolume volume;
        volume.Min = boxMin;
        volume.Max = boxMax;

        return volume;
    }
} // namespace scene
