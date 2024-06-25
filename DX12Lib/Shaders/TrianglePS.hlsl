
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

#include "LambertLighting.hlsli"

struct DirectionalLightDesc
{
    float4 direction;
    float4 color;
};

ConstantBuffer<DirectionalLightDesc> DirectionalLight : register(b1);
Texture2D Albedo : register(t1);
Texture2D NormalMap : register(t2);

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 Texture : TEXCOORD;
    float3 Tangent : TANGENT;
};

SamplerState Sampler : register(s0);

[earlydepthstencil]
float4 main(PixelShaderInput IN) : SV_Target
{
    // Sample textures
    float2 uv = IN.Texture;
    uv.y = 1 - uv.y;
    float4 textureAlbedo = Albedo.Sample(Sampler, uv);
    float4 textureNormal = NormalMap.Sample(Sampler, uv);
    
    // Calculate the TBN matrix
    float3 bitangent = cross(IN.Normal, IN.Tangent);
    float3x3 TBN = float3x3(IN.Tangent, bitangent, IN.Normal);

    // Transform the normal
    float3 finalNormal = normalize(mul(textureNormal.xyz, TBN));

    float3 color = CalculateAmbient(textureAlbedo) + CalculateDiffuse(finalNormal, IN.Color, DirectionalLight.direction.xyz, DirectionalLight.color);
    
    return textureAlbedo;
}
