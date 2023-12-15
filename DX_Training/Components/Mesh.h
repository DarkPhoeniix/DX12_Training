#pragma once

#include "IComponent.h"

struct VertexPosColor
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Color;
};

class Mesh : public IComponent
{
public:
    const std::string& GetType() const override;

    void SetVertices(const std::vector<VertexPosColor>& vertices);
    const std::vector<VertexPosColor>& GetVertices() const;

    void SetIndices(const std::vector<uint32_t>& indices);
    const std::vector<uint32_t>& GetIndices() const;

private:
    std::vector<VertexPosColor> _vertices;
    std::vector<uint32_t> _indices;
};
