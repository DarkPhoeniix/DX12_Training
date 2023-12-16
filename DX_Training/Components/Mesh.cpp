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
