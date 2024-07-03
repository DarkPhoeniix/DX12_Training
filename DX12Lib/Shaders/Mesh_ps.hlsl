
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

#include "LambertLighting.hlsli"

struct LightDesc
{
    float4 direction;
    float4 color;
};

ConstantBuffer<LightDesc> Light : register(b1);
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

SamplerState AlbedoSampler : register(s0);
SamplerState NormalSampler : register(s1);

[earlydepthstencil]
float4 main(PixelShaderInput IN) : SV_Target
{
    IN.Normal = normalize(IN.Normal);
    
    // Sample textures
    float2 uv = IN.Texture;
    uv.y = 1.0f - uv.y;
    float4 textureAlbedo = Albedo.Sample(AlbedoSampler, uv);
    float4 textureNormal = NormalMap.Sample(NormalSampler, uv);
    
    // Calculate the TBN matrix
    float3 bitangent = cross(IN.Normal, IN.Tangent);
    float3x3 TBN = float3x3(IN.Tangent, bitangent, IN.Normal);

    // Transform the normal
    float3 finalNormal = normalize(mul(textureNormal.xyz, TBN));

    float4 color = CalculateAmbient(textureAlbedo) + CalculateDiffuse(finalNormal, textureAlbedo, normalize(Light.direction.xyz), Light.color);
    
    return float4(color.rgb, textureAlbedo.w);

}
