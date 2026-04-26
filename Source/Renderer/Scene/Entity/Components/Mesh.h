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
        static constexpr std::uint8_t MAX_BONES_PER_VERTEX = 4;

        std::uint32_t BoneIds[MAX_BONES_PER_VERTEX];
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
        std::vector<std::uint32_t> IndexData;

        std::shared_ptr<rhi::Buffer> VertexBuffer;
        std::shared_ptr<rhi::Buffer> SkinningVertexBuffer;
        std::shared_ptr<rhi::Buffer> IndexBuffer;

        rhi::VertexBufferView VertexBufferView;
        rhi::VertexBufferView SkinningVertexBufferView;
        rhi::IndexBufferView IndexBufferView;
    };
} // namespace scene
