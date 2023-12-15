#include "stdafx.h"

#include "Mesh.h"

namespace
{
    const std::string& TYPE = "Mesh";
}

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
    _vertices;
}
