#include "pch.h"
#include "Node.h"

using namespace DirectX;

namespace
{
    XMMATRIX GetNodeLocalTransform(FbxNode* fbxNode)
    {
        FbxAMatrix fbxTransform = fbxNode->EvaluateLocalTransform();
        XMMATRIX transform =
        {
            (float)fbxTransform.mData[0][0], (float)fbxTransform.mData[0][1], (float)fbxTransform.mData[0][2], (float)fbxTransform.mData[0][3],
            (float)fbxTransform.mData[1][0], (float)fbxTransform.mData[1][1], (float)fbxTransform.mData[1][2], (float)fbxTransform.mData[1][3],
            (float)fbxTransform.mData[2][0], (float)fbxTransform.mData[2][1], (float)fbxTransform.mData[2][2], (float)fbxTransform.mData[2][3],
            (float)fbxTransform.mData[3][0], (float)fbxTransform.mData[3][1], (float)fbxTransform.mData[3][2], (float)fbxTransform.mData[3][3],
        };

        return transform;
    }

    std::string GetDiffuseTextureName(FbxNode* fbxNode)
    {
        std::string name;

        if (FbxSurfaceMaterial* material = fbxNode->GetMaterial(0))
        {
            FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
            if (prop.GetSrcObjectCount<FbxFileTexture>() > 0)
            {
                if (FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>(0))
                {
                    name = (const char*)(FbxPathUtils::GetFileName(texture->GetFileName()));
                }
            }
        }

        return name;
    }

    void ReadPosition(FbxMesh* fbxMesh, int polygonIndex, int vertexIndex, XMVECTOR& outPosition)
    {
        FbxVector4 position = fbxMesh->GetControlPointAt(fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex));
        outPosition = XMVectorSet((float)position.mData[0], (float)position.mData[1], (float)position.mData[2], (float)position.mData[3]);
    }

    void ReadNormal(FbxMesh* fbxMesh, int controlPointIndex, int vertexIndex, XMVECTOR& outNormal)
    {
        FbxGeometryElementNormal* vertexNormal = fbxMesh->GetElementNormal(0);
        XMFLOAT4 norm;
        switch (vertexNormal->GetMappingMode())
        {
        case FbxGeometryElement::eByControlPoint:
            switch (vertexNormal->GetReferenceMode())
            {
            case FbxGeometryElement::eDirect:
            {
                norm = {
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[0]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[1]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[2]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[3])
                };
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int index = vertexNormal->GetIndexArray().GetAt(controlPointIndex);

                norm = {
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[3])
                };
            }
            break;

            default:
                throw std::exception("Invalid Reference");
            }
            break;

        case FbxGeometryElement::eByPolygonVertex:
            switch (vertexNormal->GetReferenceMode())
            {
            case FbxGeometryElement::eDirect:
            {
                norm = {
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[0]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[1]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[2]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[3])
                };
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int index = vertexNormal->GetIndexArray().GetAt(vertexIndex);

                norm = {
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[3])
                };
            }
            break;

            default:
                throw std::exception("Invalid Reference");
            }
            break;
        }

        outNormal = XMVectorSet(norm.x, norm.y, norm.z, norm.w);
    }

    void ReadColor(fbxsdk::FbxMesh* fbxMesh, int polygonIndex, int controlPointIndex, int vertexIndex, XMVECTOR& outColor)
    {
        outColor = { 0.5f, 0.5f, 0.5f, 1.0f };

        FbxSurfaceLambert* mat = nullptr;
        for (int l = 0; l < fbxMesh->GetElementMaterialCount(); l++)
        {
            FbxGeometryElementMaterial* leVtxc = fbxMesh->GetElementMaterial(l);

            auto mapM = leVtxc->GetMappingMode();
            switch (leVtxc->GetMappingMode())
            {
            default:
                break;
            case FbxGeometryElement::eByControlPoint:
                OutputDebugStringA("eByControlPoint\n");
                switch (leVtxc->GetReferenceMode())
                {
                case FbxGeometryElement::eDirect:
                {
                    //DisplayColor(header, leVtxc->GetDirectArray().GetAt(controlPointIndex));
                    //fbxColor = leVtxc->GetDirectArray().GetAt(controlPointIndex);
                    //std::string str = "Color: " + std::to_string(fbxColor.mRed) + ", " + std::to_string(fbxColor.mGreen) + ", " + std::to_string(fbxColor.mBlue) + "\n";
                    //OutputDebugStringA(str.c_str());
                }
                break;
                case FbxGeometryElement::eIndexToDirect:
                {
                    int id = leVtxc->GetIndexArray().GetAt(controlPointIndex);
                    //DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
                    //fbxColor = leVtxc->GetDirectArray().GetAt(id);
                    //std::string str = "Color: " + std::to_string(fbxColor.mRed) + ", " + std::to_string(fbxColor.mGreen) + ", " + std::to_string(fbxColor.mBlue) + "\n";
                    //OutputDebugStringA(str.c_str());
                }
                break;
                default:
                    break; // other reference modes not shown here!
                }
                break;

            case FbxGeometryElement::eByPolygonVertex:
            {
                OutputDebugStringA("eByPolygonVertex\n");
                switch (leVtxc->GetReferenceMode())
                {
                case FbxGeometryElement::eDirect:
                {
                    //DisplayColor(header, leVtxc->GetDirectArray().GetAt(vertexId));

                    //fbxColor = leVtxc->GetDirectArray().GetAt(vertexIndex);
                    //std::string str = "Color: " + std::to_string(fbxColor.mRed) + ", " + std::to_string(fbxColor.mGreen) + ", " + std::to_string(fbxColor.mBlue) + "\n";
                    //OutputDebugStringA(str.c_str());
                }
                break;
                case FbxGeometryElement::eIndexToDirect:
                {
                    int id = leVtxc->GetIndexArray().GetAt(vertexIndex);
                    //DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
                    //fbxColor = leVtxc->GetDirectArray().GetAt(id);
                    //std::string str = "Color: " + std::to_string(fbxColor.mRed) + ", " + std::to_string(fbxColor.mGreen) + ", " + std::to_string(fbxColor.mBlue) + "\n";
                    //OutputDebugStringA(str.c_str());
                }
                break;
                default:
                    break; // other reference modes not shown here!
                }
            }
            break;

            case FbxGeometryElement::eByPolygon:
            {
                mat = (FbxSurfaceLambert*)fbxMesh->GetNode()->GetMaterial(leVtxc->GetIndexArray().GetAt(polygonIndex));
                auto amb = mat->Ambient;
                //std::string str = "Color: " + std::to_string(mat->Diffuse.Get()[0]) + ", " + std::to_string(mat->Diffuse.Get()[1]) + ", " + std::to_string(mat->Diffuse.Get()[2]) + "\n";
                //OutputDebugStringA(str.c_str());
                //OutputDebugStringA("ByPolygon\n");
                break;
            }
            case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
            {
                mat = (FbxSurfaceLambert*)fbxMesh->GetNode()->GetMaterial(0);
                auto amb = mat->Ambient;
                //OutputDebugStringA("AllSame\n");
            }
            case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                break;
            }
        }

        if (mat)
        {
            outColor = XMVectorSet(
                (float)mat->Diffuse.Get()[0],
                (float)mat->Diffuse.Get()[1],
                (float)mat->Diffuse.Get()[2],
                (float)mat->Diffuse.Get()[3]
            );
        }
    }

    void readUV(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int uvIndex, XMFLOAT2& outUV) {

        fbxsdk::FbxLayerElementUV* pFbxLayerElementUV = fbxMesh->GetLayer(0)->GetUVs();

        if (pFbxLayerElementUV == nullptr) {
            return;
        }

        switch (pFbxLayerElementUV->GetMappingMode()) {
        case FbxLayerElementUV::eByControlPoint:
        {
            switch (pFbxLayerElementUV->GetReferenceMode()) {
            case FbxLayerElementUV::eDirect:
            {
                fbxsdk::FbxVector2 fbxUv = pFbxLayerElementUV->GetDirectArray().GetAt(vertexIndex);

                outUV.x = fbxUv.mData[0];
                outUV.y = fbxUv.mData[1];

                break;
            }
            case FbxLayerElementUV::eIndexToDirect:
            {
                int id = pFbxLayerElementUV->GetIndexArray().GetAt(vertexIndex);
                fbxsdk::FbxVector2 fbxUv = pFbxLayerElementUV->GetDirectArray().GetAt(id);

                outUV.x = fbxUv.mData[0];
                outUV.y = fbxUv.mData[1];

                break;
            }
            }
            break;
        }
        case FbxLayerElementUV::eByPolygonVertex:
        {
            switch (pFbxLayerElementUV->GetReferenceMode()) {
                // Always enters this part for the example model
            case FbxLayerElementUV::eDirect:
            case FbxLayerElementUV::eIndexToDirect:
            {
                outUV.x = pFbxLayerElementUV->GetDirectArray().GetAt(uvIndex).mData[0];
                outUV.y = pFbxLayerElementUV->GetDirectArray().GetAt(uvIndex).mData[1];
                break;
            }
            }
            break;
        }
        }
    }
}

std::string Node::GetName() const
{
    return _name;
}

const std::vector<std::shared_ptr<Node>>& Node::GetChildren() const
{
    return _children;
}

DirectX::XMMATRIX Node::GetTransform() const
{
    return _transform;
}

std::pair<DirectX::XMVECTOR, DirectX::XMVECTOR> Node::GetAABB() const
{
    return _aabb;
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

std::string Node::GetTextureName() const
{
    return _textureName;
}

bool Node::Parse(FbxNode* fbxNode)
{
    _name = fbxNode->GetName();

    _transform = GetNodeLocalTransform(fbxNode);

    // Setup mesh
    if (FbxMesh* fbxMesh = fbxNode->GetMesh())
    {
        FbxVector4 min, max, center;

        fbxNode->EvaluateGlobalBoundingBoxMinMaxCenter(min, max, center);
        _aabb.first = XMVectorSet(min.mData[0], min.mData[1], min.mData[2], min.mData[3]);
        _aabb.second = XMVectorSet(max.mData[0], max.mData[1], max.mData[2], max.mData[3]);

        ParseMesh(fbxMesh);

        _textureName = GetDiffuseTextureName(fbxNode);
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

    // Save AABB data
    Json::Value jsonAABB;
    jsonAABB["Min"]["x"] = XMVectorGetX(_aabb.first);
    jsonAABB["Min"]["y"] = XMVectorGetY(_aabb.first);
    jsonAABB["Min"]["z"] = XMVectorGetZ(_aabb.first);
    jsonAABB["Min"]["w"] = XMVectorGetW(_aabb.first);
    jsonAABB["Max"]["x"] = XMVectorGetX(_aabb.second);
    jsonAABB["Max"]["y"] = XMVectorGetY(_aabb.second);
    jsonAABB["Max"]["z"] = XMVectorGetZ(_aabb.second);
    jsonAABB["Max"]["w"] = XMVectorGetW(_aabb.second);
    jsonRoot["AABB"] = jsonAABB;

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
    jsonMaterial["Diffuse"] = _textureName.c_str();

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

            ReadPosition(fbxMesh, polygonIndex, vertexIndex, position);
            ReadNormal(fbxMesh, fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex), vertexAbsoluteIndex, normal);
            ReadColor(fbxMesh, polygonIndex, fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex), vertexAbsoluteIndex, color);
            readUV(fbxMesh, vertexIndex, fbxMesh->GetTextureUVIndex(polygonIndex, vertexIndex), uv);

            _vertices.push_back(position);
            _normals.push_back(XMVector3Normalize(normal));
            _colors.push_back(color);
            _UVs.push_back(uv);
            _indices.push_back(vertexAbsoluteIndex);
        }
    }

    return false;
}
