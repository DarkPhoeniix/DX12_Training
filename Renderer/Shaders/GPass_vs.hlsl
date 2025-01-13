
#include "GPass_rootsig.hlsli"

#include "Common.hlsli"

struct VSinput
{
    float3 Position         : POSITION;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float2 Texture          : TEXCOORD;
    uint4 BoneIds           : BONE_IDS;
    float4 BoneWeights      : BONE_WEIGHTS;
};

struct VSOutput
{
    float4 WorldPosition    : POSITION;
    float4 Position         : SV_Position;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float2 Texture          : TEXCOORD;
};

struct BoneDesc
{
    row_major matrix Transform;
};

StructuredBuffer<BoneDesc> Bones : register(t1);

[RootSignature(GPass_RootSig)]
VSOutput main(VSinput IN)
{
    row_major matrix boneTransform = float4x4(
        float4(1.0f, 0.0f, 0.0f, 0.0f),
        float4(0.0f, 1.0f, 0.0f, 0.0f),
        float4(0.0f, 0.0f, 1.0f, 0.0f),
        float4(0.0f, 0.0f, 0.0f, 1.0f));
    if (Model.useSkinning)
    {
        boneTransform        = Bones[IN.BoneIds[0]].Transform * IN.BoneWeights[0];
        boneTransform       += Bones[IN.BoneIds[1]].Transform * IN.BoneWeights[1];
        boneTransform       += Bones[IN.BoneIds[2]].Transform * IN.BoneWeights[2];
        boneTransform       += Bones[IN.BoneIds[3]].Transform * IN.BoneWeights[3];
    }
    
    float4 objectPosition   = mul(float4(IN.Position, 1.0f), boneTransform);
    float3 normal           = normalize(mul(IN.Normal, (float3x3)boneTransform));
    float3 tangent          = normalize(mul(IN.Tangent, (float3x3) boneTransform));
    
    float4 worldPosition    = mul(objectPosition, Model.Transform);
    
    VSOutput output;
    output.WorldPosition    = worldPosition;
    output.Position         = mul(worldPosition, Scene.ViewProjection);
    output.Normal           = normalize(mul(normal, (float3x3) Model.Transform));
    output.Texture          = IN.Texture;
    output.Tangent          = normalize(mul(tangent, (float3x3) Model.Transform));

    return output;
}
