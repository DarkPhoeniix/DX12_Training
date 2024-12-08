
#include "GPass_rootsig.hlsli"

#include "Common.hlsli"

struct VSInput
{
    float3 Position         : POSITION;
    float3 Normal           : NORMAL;
    float4 Color            : COLOR;
    float2 Texture          : TEXCOORD;
    float3 Tangent          : TANGENT;
    uint4 BoneIds           : BONE_IDS;
    float4 BoneWeights      : BONE_WEIGHTS;
};

struct VSOutput
{
    float4 WorldPosition    : POSITION;
    float4 Position         : SV_Position;
    float3 Normal           : NORMAL;
    float4 Color            : COLOR;
    float2 Texture          : TEXCOORD;
    float3 Tangent          : TANGENT;
};

struct BoneDesc
{
    row_major matrix Transform;
};

StructuredBuffer<BoneDesc> Bones : register(t1);

[RootSignature(GPass_RootSig)]
VSOutput main(VSInput IN)
{
    row_major matrix boneTransform;
    boneTransform            = Bones[IN.BoneIds[0]].Transform * IN.BoneWeights[0];
    boneTransform           += Bones[IN.BoneIds[1]].Transform * IN.BoneWeights[1];
    boneTransform           += Bones[IN.BoneIds[2]].Transform * IN.BoneWeights[2];
    boneTransform           += Bones[IN.BoneIds[3]].Transform * IN.BoneWeights[3];
        
    float4 objectPosition   = mul(float4(IN.Position, 1.0f), boneTransform);
    float4 worldPosition    = mul(objectPosition, Model.Transform);
    
    VSOutput output;
    output.WorldPosition    = worldPosition;
    output.Position         = mul(worldPosition, Scene.ViewProjection);
    output.Normal           = normalize(mul(IN.Normal, (float3x3)Model.Transform));
    output.Color            = IN.Color;
    output.Texture          = IN.Texture;
    output.Tangent          = normalize(mul(IN.Tangent, (float3x3)Model.Transform));

    return output;
}
