#pragma once

using DirectX::XMFLOAT3;

struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT3 Color;
};

class Model
{
public:
    Model(const std::string& filepath);
    Model(const std::vector<VertexPosColor>& vertices, const std::vector<WORD>& indices);

    const std::vector<VertexPosColor> GetVertices() const;
    void SetVertices(const std::vector<VertexPosColor>& vertices);

    const std::vector<WORD> GetIndices() const;
    void SetIndices(const std::vector<WORD>& indices);

    bool ParseFile(const std::string& filepath);

private:
    std::vector<VertexPosColor> _vertices;
    std::vector<WORD> _indices;
};
