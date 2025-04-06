
#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"

struct PSinput
{
    float4 WorldPosition    : POSITION;
    float4 Position         : SV_Position;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float2 Texture          : TEXCOORD;
};

struct PSOutput
{
    float4 AlbedoMetalness  : SV_Target0;
    float4 NormalRougness   : SV_Target1;
};

StructuredBuffer<LightDesc> Lights  : register(t0);
Texture2D Materials[]               : register(t2);

SamplerState LinearSampler          : register(s0);
SamplerState PointSampler           : register(s1);

[earlydepthstencil]
PSOutput main(PSinput IN)
{
    // Sample textures
    float2 uv               = IN.Texture;
    uv.y                    = 1.0f - uv.y;
    
    float3 albedo           = Materials[Model.AlbedoTextureIndex].Sample(LinearSampler, uv).rgb;
    float3 normalMap        = Materials[Model.NormalTextureIndex].Sample(PointSampler, uv).rgb;
    float metalness         = Materials[Model.MetalnessTextureIndex].Sample(PointSampler, uv).x;
    float roughness         = Materials[Model.RoughnessTextureIndex].Sample(PointSampler, uv).x;
    roughness               = max(0.001f, roughness); // Set minimum to 0.001 to avoid some visual artifacts in PBR
    
    // Calculate the TBN matrix and a new normal vector
    float3 normal           = normalize(IN.Normal);
    float3 tangent          = normalize(IN.Tangent);
    float3 bitangent        = cross(normal, tangent);
    float3x3 TBN            = float3x3(tangent, bitangent, normal);
    
    float3 finalNormal      = normalize(mul(2.0f * normalMap - 1.0f, TBN));

    // Setup output buffer
    PSOutput output;
    output.AlbedoMetalness  = float4(albedo, metalness);
    output.NormalRougness   = float4(finalNormal, roughness);
    
    return output;
}
