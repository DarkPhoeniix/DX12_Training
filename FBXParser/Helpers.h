#pragma once

namespace FbxHelpers
{
    DirectX::XMMATRIX GetNodeLocalTransform(FbxNode* fbxNode);

    std::string GetAlbedoTextureName(FbxNode* fbxNode);
    std::string GetNormalTextureName(FbxNode* fbxNode);

    void ReadPosition(FbxMesh* fbxMesh, int polygonIndex, int vertexIndex, DirectX::XMVECTOR& outPosition);
    void ReadNormal(FbxMesh* fbxMesh, int controlPointIndex, int vertexIndex, DirectX::XMVECTOR& outNormal);
    void ReadColor(fbxsdk::FbxMesh* fbxMesh, int polygonIndex, int controlPointIndex, int vertexIndex, DirectX::XMVECTOR& outColor);
    void ReadUV(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int uvIndex, DirectX::XMFLOAT2& outUV);
    void ReadTangent(fbxsdk::FbxMesh* fbxMesh, int polygonIndex, int vertexIndex, DirectX::XMVECTOR& outTangent);
}
