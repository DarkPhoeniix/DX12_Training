#include "stdafx.h"

#include "Mesh.h"

using namespace DirectX;

namespace
{
    void readPosition(FbxMesh* fbxMesh, int polygonIndex, int vertexIndex, XMFLOAT3& outPosition)
    {
        FbxVector4 position = fbxMesh->GetControlPointAt(fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex));
        outPosition = { (float)position.mData[0], (float)position.mData[1], (float)position.mData[2] };
    }

    void readNormal(FbxMesh* fbxMesh, int controlPointIndex, int vertexIndex, XMFLOAT3& outNormal)
    {
        ASSERT((fbxMesh->GetElementNormalCount() >= 1), "Invalid normals number");

        FbxGeometryElementNormal* vertexNormal = fbxMesh->GetElementNormal(0);
        switch (vertexNormal->GetMappingMode())
        {
        case FbxGeometryElement::eByControlPoint:
            switch (vertexNormal->GetReferenceMode())
            {
            case FbxGeometryElement::eDirect:
            {
                outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[0]);
                outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[1]);
                outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[2]);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int index = vertexNormal->GetIndexArray().GetAt(controlPointIndex);
                outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
                outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
                outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
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
                outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[0]);
                outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[1]);
                outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[2]);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int index = vertexNormal->GetIndexArray().GetAt(vertexIndex);
                outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
                outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
                outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
            }
            break;

            default:
                throw std::exception("Invalid Reference");
            }
            break;
        }
    }

    void readColor(fbxsdk::FbxMesh* fbxMesh, int polygonIndex, int controlPointIndex, int vertexIndex, XMFLOAT4& outColor)
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
            outColor = { (float)mat->Diffuse.Get()[0], (float)mat->Diffuse.Get()[1], (float)mat->Diffuse.Get()[2], (float)mat->Diffuse.Get()[3] };
    }

    void ReadUV(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int uvIndex, XMFLOAT2& outUV) {

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
            float w;
            in >> v.x >> v.y >> v.z >> w;
            points.push_back({ v.x, v.y, v.z });
        }
        else if (input == "vn")
        {
            XMFLOAT3 vn;
            float w;
            in >> vn.x >> vn.y >> vn.z >> w;
            normals.push_back({ vn.x, vn.y, vn.z });
        }
        else if (input == "vc")
        {
            float r, g, b, a;
            float w;
            in >> r >> g >> b >> a;
            colors.push_back({ r, g, b, a });
        }
        else if (input == "vt")
        {
            float u, v, w;
            in >> u >> v >> w;
            UVs.push_back({ u, v });
        }
        else if (input == "f")
        {
            char sym;
            UINT64 v, vn, vt;

            for (int i = 0; i < 3; ++i)
            {
                in >> v >> sym >> vn >> sym >> vt;
                
                VertexData vertex;
                vertex.Position = points[v];
                vertex.Normal = normals[vn];
                vertex.Color = colors[v];
                vertex.UV = UVs[vt];

                _rawVertexData.push_back(vertex);
                _rawIndexData.push_back(index++);
            }
        }
    }
}
