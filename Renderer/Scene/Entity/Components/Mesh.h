#pragma once

#include "Scene/Entity/Components/IComponent.h"
#include "Scene/Volumes/AABBVolume.h"

namespace scene
{
    struct VertexData
    {
        DirectX::XMFLOAT4 Position;
        DirectX::XMFLOAT4 Normal;
        DirectX::XMFLOAT4 Tangent;
        DirectX::XMFLOAT2 UV;
    };

    struct SkinningVertexData
    {
        static constexpr uint8_t MAX_BONES_PER_VERTEX = 4;

        uint32_t BoneIds[MAX_BONES_PER_VERTEX];
        float BoneWeights[MAX_BONES_PER_VERTEX];
    };

    class Mesh : public IComponent
    {
    public:
        Mesh()
            : IComponent("Mesh")
        {
        }

        scene::AABBVolume LocalAABB;
        scene::AABBVolume GlobalAABB;

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
} // namespace scene
