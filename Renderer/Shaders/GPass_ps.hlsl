
#include "Common.hlsli"
#include "LambertLighting.hlsli"

struct PSInput
{
    float4 WorldPosition    : POSITION;
    float4 Position         : SV_Position;
    float3 Normal           : NORMAL;
    float4 Color            : COLOR;
    float2 Texture          : TEXCOORD;
    float3 Tangent          : TANGENT;
};

struct PSOutput
{
    float4 AlbedoMetalness  : SV_Target0;
    float4 NormalSpecular   : SV_Target1;
};

StructuredBuffer<LightDesc> Lights  : register(t0);
Texture2D Materials[]               : register(t1);

SamplerState LinearSampler          : register(s0);
SamplerState PointSampler           : register(s1);

[earlydepthstencil]
PSOutput main(PSInput IN)
{
    // Sample textures
    float2 uv               = IN.Texture;
    uv.y                    = 1 - uv.y;
    
    float3 albedo           = Materials[Model.AlbedoTextureIndex].Sample(LinearSampler, uv);
    float3 normalMap        = Materials[Model.NormalTextureIndex].Sample(PointSampler, uv);
    float metalness         = Materials[Model.MetalnessTextureIndex].Sample(PointSampler, uv).x;
    
    // Calculate the TBN matrix and a new normal vector
    float3 normal           = normalize(IN.Normal);
    float3 tangent          = normalize(IN.Tangent);
    float3 bitangent        = cross(normal, tangent);
    float3x3 TBN            = float3x3(tangent, bitangent, normal);
    
    float3 finalNormal      = normalize(mul(2.0f * normalMap - 1.0f, TBN));

    // Setup output buffer
    PSOutput output;
    output.AlbedoMetalness  = float4(albedo, 1.0f);
    output.NormalSpecular   = float4(finalNormal, 1.0f);
    
    return output;
}
