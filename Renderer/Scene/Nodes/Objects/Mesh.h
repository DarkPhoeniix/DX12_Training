#pragma once

struct VertexData
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT4 Color;
    DirectX::XMFLOAT2 UV;
    DirectX::XMFLOAT3 Tangent;
};

namespace SceneLayer
{
    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() = default;

        const std::vector<VertexData>& getVertices() const;
        const std::vector<UINT>& getIndices() const;

        void LoadMesh(const std::string& filepath);

    private:
        std::vector<VertexData> _rawVertexData;
        std::vector<UINT> _rawIndexData;
    };
} // namespace SceneLayer
