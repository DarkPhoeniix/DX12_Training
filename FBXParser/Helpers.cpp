#include "pch.h"

#include "Helpers.h"

using namespace DirectX;

namespace FbxHelpers
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

    std::string GetAlbedoTextureName(FbxNode* fbxNode)
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

    std::string GetNormalTextureName(FbxNode* fbxNode)
    {
        std::string name;

        if (FbxSurfaceMaterial* material = fbxNode->GetMaterial(0))
        {
            FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sNormalMap);
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

    void ReadTangent(fbxsdk::FbxMesh* fbxMesh, int controlPointIndex, int vertexIndex, DirectX::XMVECTOR& outTangent)
    {
        if (FbxLayerElementTangent* pFbxLayerElementTangent = fbxMesh->GetLayer(0)->GetTangents())
        {
            XMFLOAT4 tangent;

            switch (pFbxLayerElementTangent->GetMappingMode())
            {
            case FbxGeometryElement::eByControlPoint:
                switch (pFbxLayerElementTangent->GetReferenceMode())
                {
                case FbxGeometryElement::eDirect:
                {
                    tangent = {
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(controlPointIndex).mData[0]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(controlPointIndex).mData[1]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(controlPointIndex).mData[2]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(controlPointIndex).mData[3])
                    };
                }
                break;

                case FbxGeometryElement::eIndexToDirect:
                {
                    int index = pFbxLayerElementTangent->GetIndexArray().GetAt(controlPointIndex);

                    tangent = {
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(index).mData[0]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(index).mData[1]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(index).mData[2]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(index).mData[3])
                    };
                }
                break;

                default:
                    throw std::exception("Invalid Reference");
                }
                break;

            case FbxGeometryElement::eByPolygonVertex:
                switch (pFbxLayerElementTangent->GetReferenceMode())
                {
                case FbxGeometryElement::eDirect:
                {
                    tangent = {
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(vertexIndex).mData[0]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(vertexIndex).mData[1]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(vertexIndex).mData[2]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(vertexIndex).mData[3])
                    };
                }
                break;

                case FbxGeometryElement::eIndexToDirect:
                {
                    int index = pFbxLayerElementTangent->GetIndexArray().GetAt(vertexIndex);

                    tangent = {
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(index).mData[0]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(index).mData[1]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(index).mData[2]),
                        static_cast<float>(pFbxLayerElementTangent->GetDirectArray().GetAt(index).mData[3])
                    };
                }
                break;

                default:
                    throw std::exception("Invalid Reference");
                }
                break;
            }

            outTangent = XMVectorSet(tangent.x, tangent.y, tangent.z, tangent.w);
        }
    }
}
