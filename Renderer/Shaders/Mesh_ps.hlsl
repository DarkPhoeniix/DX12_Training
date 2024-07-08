
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

#include "LambertLighting.hlsli"

struct LightsDesc
{
    uint DirectionalLightNum;
    uint PointLightNum;
};

struct DirectionalLightDesc
{
    float4 Direction;
    float4 Color;
};

struct PointLightDesc
{
    float4 Position;
    float4 Color;
    float Range;
    float Intensity;
};

struct MaterialDesc
{
    Texture2D Albedo;
    Texture2D Normal;
};

ConstantBuffer<LightsDesc> LightsDesc_CB : register(b1);

StructuredBuffer<DirectionalLightDesc> DirectionalLights_SB : register(t1);
StructuredBuffer<PointLightDesc> PointLights_SB : register(t2);

Texture2D Albedo : register(t3);
Texture2D Normal : register(t4);

SamplerState AlbedoSampler : register(s0);
SamplerState NormalSampler : register(s1);

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 Texture : TEXCOORD;
    float3 Tangent : TANGENT;
};

[earlydepthstencil]
float4 main(PixelShaderInput IN) : SV_Target
{
    IN.Normal = normalize(IN.Normal);
    
    // Sample textures
    float2 uv = IN.Texture;
    uv.y = 1.0f - uv.y;
    float4 textureAlbedo = Albedo.Sample(AlbedoSampler, uv);
    float4 textureNormal = Normal.Sample(NormalSampler, uv);
    
    // Calculate the TBN matrix
    float3 bitangent = cross(IN.Normal, IN.Tangent);
    float3x3 TBN = float3x3(IN.Tangent, bitangent, IN.Normal);

    // Transform the normal
    float3 finalNormal = normalize(mul(textureNormal.xyz, TBN));

    float4 color  = CalculateAmbient(textureAlbedo) + 
                    CalculateDiffuse(finalNormal, textureAlbedo, normalize(DirectionalLights_SB[0].Direction.xyz), DirectionalLights_SB[0].Color);
    
    return float4(color.rgb, textureAlbedo.w);

}
