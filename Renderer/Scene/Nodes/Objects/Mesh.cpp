#include "stdafx.h"

#include "Mesh.h"

using namespace DirectX;

namespace SceneLayer
{
    const std::vector<VertexData>& Mesh::getVertices() const
    {
        return _rawVertexData;
    }

    const std::vector<UINT>& Mesh::getIndices() const
    {
        return _rawIndexData;
    }

    void Mesh::LoadMesh(const std::string& filepath)
    {
        std::vector<XMFLOAT3> points;
        std::vector<XMFLOAT3> normals;
        std::vector<XMFLOAT4> colors;
        std::vector<XMFLOAT2> UVs;
        std::vector<XMFLOAT3> tangents;
        UINT64 index = 0;

        std::string input;
        std::ifstream in(filepath, std::ios_base::in);
        while (!in.eof())
        {
            input = "";
            in >> input;

            if (input == "")
            {
                char line[512];
                in.getline(line, 512);
            }
            else if (input == "v")
            {
                XMFLOAT3 v;
                in >> v.x >> v.y >> v.z;
                points.push_back(v);
            }
            else if (input == "vn")
            {
                XMFLOAT3 vn;
                in >> vn.x >> vn.y >> vn.z;
                normals.push_back(vn);
            }
            else if (input == "vc")
            {
                float r, g, b, a;
                in >> r >> g >> b >> a;
                colors.push_back({ r, g, b, a });
            }
            else if (input == "vt")
            {
                float u, v;
                in >> u >> v;
                UVs.push_back({ u, v });
            }
            else if (input == "vtan")
            {
                XMFLOAT3 tangent;
                in >> tangent.x >> tangent.y >> tangent.z;
                tangents.push_back(tangent);
            }
            else if (input == "f")
            {
                char sym;
                UINT64 v, vn, vt, vtan;

                for (int i = 0; i < 3; ++i)
                {
                    in >> v >> sym >> vt >> sym >> vn >> sym >> vtan;

                    VertexData vertex;
                    vertex.Position = points[v];
                    vertex.Normal = normals[vn];
                    vertex.Color = { 0.8f, 0.8f, 0.8f, 1.0f };
                    vertex.UV = UVs[vt];
                    vertex.Tangent = tangents[vtan];

                    _rawVertexData.push_back(vertex);
                    _rawIndexData.push_back(index++);
                }
            }
        }
    }
} // namespace SceneLayer
