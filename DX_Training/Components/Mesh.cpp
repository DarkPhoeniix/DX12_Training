#include "stdafx.h"

#include "Mesh.h"

namespace
{
    const std::string& TYPE = "Mesh";
}

Register_Component(Mesh);

const std::string& Mesh::GetType() const
{
    return TYPE;
}

void Mesh::SetVertices(const std::vector<VertexPosColor>& vertices)
{
    _vertices = vertices;
}

const std::vector<VertexPosColor>& Mesh::GetVertices() const
{
    return _vertices;
}

void Mesh::SetIndices(const std::vector<uint32_t>& indices)
{
    _indices = indices;
}

const std::vector<uint32_t>& Mesh::GetIndices() const
{
    return _indices;
}
