#pragma once

#include "Scene/Entity/Components/IComponent.h"
#include "Scene/Volumes/AABBVolume.h"

namespace SceneLayer
{
    struct VertexData
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT3 Tangent;
        DirectX::XMFLOAT2 UV;
    };

    struct SkinningVertexData
    {
        static constexpr uint8_t MAX_BONES_PER_VERTEX = 4;

        uint32_t BoneIds[MAX_BONES_PER_VERTEX];
        float BoneWeights[MAX_BONES_PER_VERTEX];
    };

    struct Mesh : public IComponent
    {
        Mesh()
            : IComponent("Mesh")
        {
        }

        SceneLayer::AABBVolume AABB;

        std::vector<VertexData> VertexData;
        std::vector<SkinningVertexData> SkinningVertexData;
        std::vector<UINT> IndexData;

        std::shared_ptr<dx12::Resource> VertexBuffer;
        std::shared_ptr<dx12::Resource> SkinningVertexBuffer;
        std::shared_ptr<dx12::Resource> IndexBuffer;

        D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
        D3D12_VERTEX_BUFFER_VIEW SkinningVertexBufferView;
        D3D12_INDEX_BUFFER_VIEW IndexBufferView;
    };
} // namespace SceneLayer
