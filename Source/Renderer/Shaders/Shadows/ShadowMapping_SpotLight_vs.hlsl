
#include "ShadowMapping_rootsig.hlsli"

#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"

struct VSinput
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float2 Texture      : TEXCOORD;
    uint4  BoneIds      : BONE_IDS;
    float4 BoneWeights  : BONE_WEIGHTS;
};

struct VSOutput
{
    float4 Position     : SV_Position;
};

struct BoneDesc
{
    row_major matrix Transform;
};

struct ShadowData
{
    uint LightIndex;
};

ConstantBuffer<SceneDesc> Scene : register(b0);
ConstantBuffer<ModelDesc> Model : register(b1);
ConstantBuffer<ShadowData>  Shadow  : register(b3);
StructuredBuffer<BoneDesc>  Bones   : register(t0);
StructuredBuffer<LightDesc> Lights  : register(t1);

[RootSignature(ShadowMapping_RootSig)]
VSOutput main(VSinput IN)
{
    row_major matrix boneTransform = float4x4(
        float4(1.0f, 0.0f, 0.0f, 0.0f),
        float4(0.0f, 1.0f, 0.0f, 0.0f),
        float4(0.0f, 0.0f, 1.0f, 0.0f),
        float4(0.0f, 0.0f, 0.0f, 1.0f));
    if (Model.BonesBufferIndex != -1)
    {
        boneTransform = Bones[IN.BoneIds[0]].Transform * IN.BoneWeights[0];
        boneTransform += Bones[IN.BoneIds[1]].Transform * IN.BoneWeights[1];
        boneTransform += Bones[IN.BoneIds[2]].Transform * IN.BoneWeights[2];
        boneTransform += Bones[IN.BoneIds[3]].Transform * IN.BoneWeights[3];
    }
    
    float4 positionWS   = float4(IN.Position, 1.0f);
    positionWS          = mul(positionWS, boneTransform);
    positionWS          = mul(positionWS, Model.Transform);
    float4 positionLS   = mul(positionWS, Lights[Shadow.LightIndex].ViewProj[0]);
    
    VSOutput output;
    output.Position = positionLS;
    
    return output;
}
