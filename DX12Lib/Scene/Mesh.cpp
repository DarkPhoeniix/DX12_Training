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

Mesh::Mesh(FbxMesh* fbxMesh)
{
    size_t verticesCount = fbxMesh->GetControlPointsCount();
    _rawVertexData.reserve(verticesCount);

    {
        size_t polygonCount = fbxMesh->GetPolygonCount();
        size_t indexCount = polygonCount * 3; // polygons can be only triangles
        _rawIndexData.reserve(indexCount);

        for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
        {
            for (int vertexIndex = 0; vertexIndex < fbxMesh->GetPolygonSize(polygonIndex); ++vertexIndex)
            {
                int vertexAbsoluteIndex = polygonIndex * 3 + vertexIndex;
                VertexData vertex;

                readPosition(fbxMesh, polygonIndex, vertexIndex, vertex.Position);
                readNormal(fbxMesh, fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex), vertexAbsoluteIndex, vertex.Normal);
                readColor(fbxMesh, polygonIndex, fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex), vertexAbsoluteIndex, vertex.Color);
                readUV(fbxMesh, vertexIndex, fbxMesh->GetTextureUVIndex(polygonIndex, vertexIndex), vertex.UV);

                _rawVertexData.push_back(vertex);
                _rawIndexData.push_back(vertexAbsoluteIndex);
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
