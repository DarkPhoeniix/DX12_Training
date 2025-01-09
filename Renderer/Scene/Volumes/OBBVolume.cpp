#include "RendererPCH.h"

#include "OBBVolume.h"

#include "AABBVolume.h"

namespace
{
    const static DirectX::XMVECTOR _kBoxVerts[8] =
    {
        // front rect
        DirectX::XMVectorSet(-1.0f, -1.0f, -1.0f, 1.0f),
        DirectX::XMVectorSet(-1.0f,  1.0f, -1.0f, 1.0f),
        DirectX::XMVectorSet(1.0f,  1.0f, -1.0f, 1.0f),
        DirectX::XMVectorSet(1.0f, -1.0f, -1.0f, 1.0f),

        // back rect
        DirectX::XMVectorSet(-1.0f, -1.0f,  1.0f, 1.0f),
        DirectX::XMVectorSet(-1.0f,  1.0f,  1.0f, 1.0f),
        DirectX::XMVectorSet(1.0f,  1.0f,  1.0f, 1.0f),
        DirectX::XMVectorSet(1.0f, -1.0f,  1.0f, 1.0f)
    };
} // namespace unnamed

namespace SceneLayer
{
    AABBVolume CombineOBBs(const std::vector<OBBVolume>& volumes)
    {
        AABBVolume result;

        for (const auto& volume : volumes)
        {
            if (!DirectX::XMMatrixIsNaN(volume.Bounds))
            {
                for (int i = 0; i < 8; ++i)
                {
                    DirectX::XMVECTOR v = DirectX::XMVector4Transform(_kBoxVerts[i], volume.Bounds);
                    result.Min = DirectX::XMVectorMin(result.Min, v);
                    result.Max = DirectX::XMVectorMax(result.Max, v);
                }
            }
        }

        return result;
    }
} // namespace SceneLayer
