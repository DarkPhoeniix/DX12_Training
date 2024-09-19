
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

#include "Common.hlsli"
#include "LambertLighting.hlsli"

struct LightDesc
{
    float4 Direction;
    float4 Color;
};

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
    float2 Texture  : TEXCOORD;
    float3 Tangent  : TANGENT;
};

StructuredBuffer<LightDesc> Lights  : register(t0);
Texture2D Materials[]               : register(t1);

SamplerState AlbedoSampler          : register(s0);
SamplerState PointSampler           : register(s1);

float4 main(PixelShaderInput IN) : SV_Target
{
    IN.Normal = normalize(IN.Normal);
    
    // Sample textures
    float2 uv = IN.Texture;
    uv.y = 1.0f - uv.y;
    float4 textureAlbedo    = Materials[Model.AlbedoTextureIndex].Sample(AlbedoSampler, uv);
    float4 textureNormal    = Materials[Model.NormalTextureIndex].Sample(PointSampler, uv);
    float4 textureMetalness = Materials[Model.MetalnessTextureIndex].Sample(PointSampler, uv);
    
    // Calculate the TBN matrix
    float3 bitangent = cross(IN.Normal, IN.Tangent);
    float3x3 TBN = float3x3(IN.Tangent, bitangent, IN.Normal);

    // Transform the normal
    float3 finalNormal = normalize(mul(textureNormal.xyz, TBN));

    float4 ambient = CalculateAmbient(textureAlbedo);
    float4 diffuse = CalculateDiffuse(finalNormal, textureAlbedo, normalize(Lights[0].Direction.xyz), Lights[0].Color);
    
    float4 color = ambient + diffuse;
    
    return float4(color.rgb, textureAlbedo.w);
}
