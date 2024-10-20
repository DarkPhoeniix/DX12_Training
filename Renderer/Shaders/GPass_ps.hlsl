
#include "Common.hlsli"
#include "LambertLighting.hlsli"

struct PSInput
{
    float4 WorldPosition    : POSITION;
    float4 Position         : SV_Position;
    float3 Normal           : NORMAL; // should be normalized
    float4 Color            : COLOR;
    float2 Texture          : TEXCOORD;
    float3 Tangent          : TANGENT;
};

struct PSOutput
{
    float4 AlbedoMetalness  : SV_Target0;
    float4 NormalSpecular   : SV_Target1;
};

StructuredBuffer<LightDesc> Lights : register(t0);
Texture2D Materials[] : register(t1);

SamplerState LinearSampler : register(s0);
SamplerState PointSampler : register(s1);

PSOutput main(PSInput IN)
{
    // Sample textures
    float2 uv = IN.Texture;
    uv.y = 1 - uv.y;
    
    // Calculate the TBN matrix
    float4 normal = Materials[Model.NormalTextureIndex].Sample(PointSampler, uv);
    float3 bitan = cross(IN.Normal, IN.Tangent);
    float3x3 TBN = float3x3(IN.Tangent, bitan, IN.Normal);

    // Setup surface
    float3 albedo = Materials[Model.AlbedoTextureIndex].Sample(LinearSampler, uv);
    float metalness = Materials[Model.MetalnessTextureIndex].Sample(LinearSampler, uv).x;
    float3 finalNormal = mul(2.0f * normal.xyz - 1.0f, TBN);
    
    PSOutput output;
    
    //output.Position = IN.WorldPosition;
    output.AlbedoMetalness = float4(albedo, 1.0f);
    output.NormalSpecular = float4(finalNormal, 1.0f);
    
    return output;
}
