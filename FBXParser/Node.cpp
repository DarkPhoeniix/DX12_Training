#include "pch.h"

#include "Node.h"

#include "Helpers.h"

using namespace DirectX;

std::string Node::GetName() const
{
    return _name;
}

DirectX::XMMATRIX Node::GetTransform() const
{
    return _transform;
}

const std::vector<std::shared_ptr<Node>>& Node::GetChildren() const
{
    return _children;
}

const std::vector<DirectX::XMVECTOR>& Node::GetVertices() const
{
    return _vertices;
}

const std::vector<DirectX::XMVECTOR>& Node::GetNormals() const
{
    return _normals;
}

const std::vector<DirectX::XMVECTOR>& Node::GetColors() const
{
    return _colors;
}

const std::vector<DirectX::XMFLOAT2>& Node::GetUVs() const
{
    return _UVs;
}

const std::vector<UINT64>& Node::GetIndices() const
{
    return _indices;
}

std::string Node::GetAlbedoTextureName() const
{
    return _albedoTextureName;
}

std::string Node::GetNormalTextureName() const
{
    return _normalTextureName;
}

bool Node::Parse(FbxNode* fbxNode)
{
    _name = fbxNode->GetName();

    _transform = FbxHelpers::GetNodeLocalTransform(fbxNode);

    // Setup mesh
    if (FbxMesh* fbxMesh = fbxNode->GetMesh())
    {
        ParseMesh(fbxMesh);

        _albedoTextureName = FbxHelpers::GetAlbedoTextureName(fbxNode);
        _normalTextureName = FbxHelpers::GetNormalTextureName(fbxNode);
    }

    // Setup child nodes
    for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
    {
        auto childNode = std::make_shared<Node>();
        childNode->Parse(fbxNode->GetChild(childIndex));
        _children.push_back(childNode);
    }

    return true;
}

bool Node::Save(const std::string& path) const
{
    std::string rootPath = path + _name + ".node";
    std::ofstream out(rootPath, std::fstream::out | std::ios_base::binary);

    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    Json::Value jsonRoot;
    Json::Value nodes(Json::arrayValue);

    jsonRoot["Name"] = _name.c_str();

    if (!_vertices.empty())
    {
        // Save mesh data
        std::string meshFilepath = (_name + ".mesh").c_str();
        jsonRoot["Mesh"] = meshFilepath.c_str();
        SaveMesh(path + meshFilepath);

        // Save material data
        std::string materialFilepath = (_name + ".mat").c_str();
        jsonRoot["Material"] = materialFilepath.c_str();
        SaveMaterial(path + materialFilepath);
    }

    Json::Value jsonTransform;
    jsonTransform["r0"]["x"] = XMVectorGetX(_transform.r[0]);
    jsonTransform["r0"]["y"] = XMVectorGetY(_transform.r[0]);
    jsonTransform["r0"]["z"] = XMVectorGetZ(_transform.r[0]);
    jsonTransform["r0"]["w"] = XMVectorGetW(_transform.r[0]);

    jsonTransform["r1"]["x"] = XMVectorGetX(_transform.r[1]);
    jsonTransform["r1"]["y"] = XMVectorGetY(_transform.r[1]);
    jsonTransform["r1"]["z"] = XMVectorGetZ(_transform.r[1]);
    jsonTransform["r1"]["w"] = XMVectorGetW(_transform.r[1]);

    jsonTransform["r2"]["x"] = XMVectorGetX(_transform.r[2]);
    jsonTransform["r2"]["y"] = XMVectorGetY(_transform.r[2]);
    jsonTransform["r2"]["z"] = XMVectorGetZ(_transform.r[2]);
    jsonTransform["r2"]["w"] = XMVectorGetW(_transform.r[2]);

    jsonTransform["r3"]["x"] = XMVectorGetX(_transform.r[3]);
    jsonTransform["r3"]["y"] = XMVectorGetY(_transform.r[3]);
    jsonTransform["r3"]["z"] = XMVectorGetZ(_transform.r[3]);
    jsonTransform["r3"]["w"] = XMVectorGetW(_transform.r[3]);
    jsonRoot["Transform"] = jsonTransform;

    for (const auto& node : _children)
    {
        node->Save(path);
        nodes.append((node->GetName() + ".node").c_str());
    }
    jsonRoot["Nodes"] = nodes;

    writer->write(jsonRoot, &out);


    return false;
}

bool Node::SaveChildren(const std::string& path) const
{
    return false;
}

bool Node::SaveMesh(const std::string& path) const
{
    std::ofstream out(path, std::fstream::out);

    for (const auto& vertex : _vertices)
    {
        out << "v " << XMVectorGetX(vertex) << ' ' << XMVectorGetY(vertex) << ' ' << XMVectorGetZ(vertex) << ' ' << XMVectorGetW(vertex) << '\n';
    }

    for (const auto& normal : _normals)
    {
        out << "vn " << XMVectorGetX(normal) << ' ' << XMVectorGetY(normal) << ' ' << XMVectorGetZ(normal) << ' ' << XMVectorGetW(normal) << '\n';
    }

    for (const auto& color : _colors)
    {
        out << "vc " << XMVectorGetX(color) << ' ' << XMVectorGetY(color) << ' ' << XMVectorGetZ(color) << ' ' << 1.0f << '\n';
    }

    for (const auto& uv : _UVs)
    {
        out << "vt " << uv.x << ' ' << uv.y << " 0" << '\n';
    }

    for (int i = 0; i < _indices.size(); i += 3)
    {
        out << "f " <<
            _indices[i] << '/' << _indices[i] << '/' << _indices[i] << ' ' <<
            _indices[i + 1] << '/' << _indices[i + 1] << '/' << _indices[i + 1] << ' ' <<
            _indices[i + 2] << '/' << _indices[i + 2] << '/' << _indices[i + 2] << '\n';
    }

    return true;
}

bool Node::SaveMaterial(const std::string& path) const
{
    std::ofstream out(path, std::fstream::out | std::ios_base::binary);

    Json::Value jsonMaterial;
    jsonMaterial["Albedo"] = _albedoTextureName.c_str();
    jsonMaterial["Normal"] = _normalTextureName.c_str();

    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(jsonMaterial, &out);

    return true;
}

bool Node::ParseMesh(FbxMesh* fbxMesh)
{
    for (int polygonIndex = 0; polygonIndex < fbxMesh->GetPolygonCount(); ++polygonIndex)
    {
        for (int vertexIndex = 0; vertexIndex < fbxMesh->GetPolygonSize(polygonIndex); ++vertexIndex)
        {
            int vertexAbsoluteIndex = polygonIndex * 3 + vertexIndex;

            XMVECTOR position;
            XMVECTOR normal;
            XMVECTOR color;
            XMFLOAT2 uv;

            FbxHelpers::ReadPosition(fbxMesh, polygonIndex, vertexIndex, position);
            FbxHelpers::ReadNormal(fbxMesh, fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex), vertexAbsoluteIndex, normal);
            FbxHelpers::ReadColor(fbxMesh, polygonIndex, fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex), vertexAbsoluteIndex, color);
            FbxHelpers::ReadUV(fbxMesh, vertexIndex, fbxMesh->GetTextureUVIndex(polygonIndex, vertexIndex), uv);

            _vertices.push_back(position);
            _normals.push_back(XMVector3Normalize(normal));
            _colors.push_back(color);
            _UVs.push_back(uv);
            _indices.push_back(vertexAbsoluteIndex);
        }
    }

    return false;
}
