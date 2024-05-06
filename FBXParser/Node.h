#pragma once

class Node
{
public:
    std::string GetName() const;

    const std::vector<std::shared_ptr<Node>>& GetChildren() const;

    DirectX::XMMATRIX GetTransform() const;
    std::pair<DirectX::XMVECTOR, DirectX::XMVECTOR> GetAABB() const;

    const std::vector<DirectX::XMVECTOR>& GetVertices() const;
    const std::vector<DirectX::XMVECTOR>& GetNormals() const;
    const std::vector<DirectX::XMVECTOR>& GetColors() const;
    const std::vector<DirectX::XMFLOAT2>& GetUVs() const;
    const std::vector<UINT64>& GetIndices() const;

    std::string GetTextureName() const;

    bool Parse(FbxNode* fbxNode);

    bool Save(const std::string& path) const;

private:
    bool ParseMesh(FbxMesh* fbxMesh);

    bool SaveChildren(const std::string& path) const;
    bool SaveMesh(const std::string& path) const;
    bool SaveMaterial(const std::string& path) const;

    std::string _name;

    std::vector<std::shared_ptr<Node>> _children;

    DirectX::XMMATRIX _transform;
    std::pair<DirectX::XMVECTOR, DirectX::XMVECTOR> _aabb;

    std::vector<DirectX::XMVECTOR> _vertices;
    std::vector<DirectX::XMVECTOR> _normals;
    std::vector<DirectX::XMVECTOR> _colors;
    std::vector<DirectX::XMFLOAT2> _UVs;
    std::vector<UINT64> _indices;

    std::string _textureName;
};
