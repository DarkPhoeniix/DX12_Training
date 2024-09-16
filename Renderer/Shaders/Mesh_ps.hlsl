
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

#include "LambertLighting.hlsli"

struct SceneDesc
{
    float4 eyePosition;
    float4 eyeDirection;
    
    uint directionalLightsNum;
    uint pointLightsNum;
};

struct LightDesc
{
    float4 direction;
    float4 color;
};

struct ModelDesc
{
    uint AlbedoTextureIndex;
    uint NormalTextureIndex;
    uint MetalnessTextureIndex;
};

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
    float2 Texture  : TEXCOORD;
    float3 Tangent  : TANGENT;
};

ConstantBuffer<SceneDesc> Scene : register(b1);
ConstantBuffer<LightDesc> Light : register(b2);

ModelDesc Model                 : register(t1);
Texture2D Materials[]           : register(t2);

SamplerState AlbedoSampler      : register(s0);
SamplerState PointSampler       : register(s1);

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
    float4 diffuse = CalculateDiffuse(finalNormal, textureAlbedo, normalize(Light.direction.xyz), Light.color);
    
    float4 color = ambient + diffuse;
    
    return float4(color.rgb, textureAlbedo.w);
}
