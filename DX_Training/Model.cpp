#include "stdafx.h"
#include "Model.h"

#include <unordered_map>

namespace
{
    std::list<std::string> getLines(const std::string& filename)
    {
        std::ifstream inputStream(filename);

        if (!inputStream.is_open())
        {
            throw std::invalid_argument("Can't open file for reading: " + filename);
        }

        std::list<std::string> lines;
        std::string line;

        while (std::getline(inputStream, line))
        {
            if (!line.empty())
            {
                lines.push_back(line);
            }
        }

        return lines;
    }

    DirectX::XMVECTOR parseVertex(const std::string& line)
    {
        DirectX::XMVECTOR vertex;
        std::istringstream stream(line.substr(2));

        float x, y, z;
        stream >> x >> y >> z;

        return DirectX::XMVectorSet(x, y, z, 1.0f);
    }

    std::vector<unsigned int> parseIndices(const std::string& line)
    {
        std::vector<unsigned int> indices;
        unsigned int index;
        std::istringstream stream(line.substr(2));

        for (size_t i = 0; i < 3; ++i)
        {
            stream >> index;
            indices.push_back(index);
        }

        return indices;
    }
}

Model::Model(const std::string& filepath)
{
    ParseFile(filepath);
}

Model::Model(const std::vector<VertexPosColor>& vertices, const std::vector<WORD>& indices)
{
    _vertices = vertices;
    _indices = indices;
}

const std::vector<VertexPosColor> Model::GetVertices() const
{
    return _vertices;
}

void Model::SetVertices(const std::vector<VertexPosColor>& vertices)
{
    _vertices = vertices;
}

const std::vector<WORD> Model::GetIndices() const
{
    return _indices;
}

void Model::SetIndices(const std::vector<WORD>& indices)
{
    _indices = indices;
}

bool Model::ParseFile(const std::string& filepath)
{
    std::ifstream fin(filepath);

    std::vector<XMFLOAT3> vertices;
    std::vector<XMFLOAT3> normals;
    //std::vector<float2> textures;

    std::unordered_map< long long, int > avilableIndices;
    std::vector<long long> indices;

    int nCurrentIndex = 0;

    if (!fin)
    {
        return false;
    }


    std::string type;
    char line[512];

    while (!fin.eof())
    {
        fin >> type; // read type of
        if (type == "#" || type.empty())
        {
            // comments, just skip the line
            fin.getline(line, 512);
        }
        else if (type == "v")
        {
            // read vertex
            XMFLOAT3 ver;
            fin >> ver.x >> ver.y >> ver.z;
            vertices.push_back(ver);
        }
        else if (type == "vn")
        {
            // read normal
            XMFLOAT3 ver;
            fin >> ver.x >> ver.y >> ver.z;
            normals.push_back(ver);
        }
        else if (type == "vt")
        {
            fin.getline(line, 512);
        }
        else if (type == "g")
        {
            // i don't know what the fuck is that, just skip
            fin.getline(line, 512);
        }
        else if (type == "f")
        {
            // read index
            unsigned long value[3];

            char symbol;

            for (int i = 0; i < 3; ++i)
            {
                fin >> value[0] >> symbol >> value[1] >> symbol >> value[2];

                value[0] -= 1;
                value[1] -= 1;
                value[2] -= 1;

                long long dwValue =
                    (unsigned short)value[0] << 0 |
                    (unsigned int)value[1] << 16 |
                    (long long)value[2] << 48;

                //char buf[ 64 ];
                //sprintf( buf, "%i, %i, %i \n", value[ 0 ], value[ 1 ], value[ 2 ] );
                //OutputDebugStringA( buf );

                indices.push_back(dwValue);

                std::unordered_map< long long, int >::iterator itFind = avilableIndices.find(dwValue);
                if (itFind == avilableIndices.end())
                {
                    avilableIndices[dwValue] = nCurrentIndex;
                    ++nCurrentIndex;
                }
            }
        }
        else
        {
            fin.getline(line, 512);
        }
    }

    {
        // fill vertex data
        //
        _vertices.clear();
        _vertices.resize(avilableIndices.size());

        for (std::unordered_map< long long, int >::const_iterator itStart = avilableIndices.begin(), itEnd = avilableIndices.end();
            itStart != itEnd; ++itStart)
        {
            long long data = itStart->first;
            unsigned long value[3];
            value[0] = (unsigned short)data >> 0;// 0x00000000ffff;
            value[1] = (unsigned int)data >> 16;// 0x0000ffff0000;
            value[2] = (long long)data >> 48;// 0xffff00000000;

            //DataMesh dataVert;
            //dataVert.Position = vertices[ value[ 0 ] ];
            //dataVert.Texture = textures[ value[ 1 ] ];
            //dataVert.Normal = normals[ value[ 2 ] ];
            //verticesResult.push_back( std::move( dataVert ) );


            auto& dataVert = _vertices[itStart->second];
            dataVert.Position = vertices[value[0]];
            //dataVert.uv = textures[value[1]];
            dataVert.Normal = normals[value[2]];
            dataVert.Color = { 1.0f, 1.0f, 1.0f };
        }

        // generate indecies
        _indices.clear();
        _indices.reserve(indices.size());
        for (std::vector< long long >::const_iterator itStart = indices.begin(), itEnd = indices.end(); itStart != itEnd; ++itStart)
        {
            long long data = *itStart;

            std::unordered_map< long long, int >::const_iterator itFind = avilableIndices.find(data);
            _indices.push_back(itFind->second);
        }
    }

    return true;
}
