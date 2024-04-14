#pragma once

#include <fbxsdk.h>

struct VertexData
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT4 Color;
    DirectX::XMFLOAT2 UV;
};

class Mesh
{
public:
    Mesh(FbxMesh* fbxMesh);

    const std::vector<VertexData>& getVertices() const;
    const std::vector<UINT>& getIndices() const;

private:
    std::vector<VertexData> _rawVertexData;
    std::vector<UINT> _rawIndexData;
};
