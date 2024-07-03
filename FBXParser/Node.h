#pragma once

class Node
{
public:
    std::string GetName() const;

    DirectX::XMMATRIX GetTransform() const;
    const std::vector<std::shared_ptr<Node>>& GetChildren() const;

    const std::vector<DirectX::XMVECTOR>& GetVertices() const;
    const std::vector<DirectX::XMVECTOR>& GetNormals() const;
    const std::vector<DirectX::XMVECTOR>& GetColors() const;
    const std::vector<DirectX::XMFLOAT2>& GetUVs() const;
    const std::vector<UINT64>& GetIndices() const;

    std::string GetAlbedoTextureName() const;
    std::string GetNormalTextureName() const;

    bool Parse(FbxNode* fbxNode);

    bool Save(const std::string& path) const;

private:
    bool ParseMesh(FbxMesh* fbxMesh);

    bool SaveChildren(const std::string& path) const;
    bool SaveMesh(const std::string& path) const;
    bool SaveMaterial(const std::string& path) const;

    std::string _name;

    DirectX::XMMATRIX _transform;
    std::vector<std::shared_ptr<Node>> _children;

    std::vector<DirectX::XMVECTOR> _vertices;
    std::vector<DirectX::XMVECTOR> _normals;
    std::vector<DirectX::XMVECTOR> _colors;
    std::vector<DirectX::XMFLOAT2> _UVs;
    std::vector<DirectX::XMVECTOR> _tangents;
    std::vector<UINT64> _indices;

    std::string _albedoTextureName;
    std::string _normalTextureName;
};
