
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

#include "Common.hlsli"
#include "LambertLighting.hlsli"

struct PixelShaderInput
{
    float4 WorldPosition : POSITION;
    float4 Position : SV_Position;
    float3 Normal   : NORMAL;   // should be normalized
    float4 Color    : COLOR;
    float2 Texture  : TEXCOORD;
    float3 Tangent  : TANGENT;
};  

StructuredBuffer<LightDesc> Lights  : register(t0);
Texture2D Materials[]               : register(t1);

SamplerState LinearSampler          : register(s0);
SamplerState PointSampler           : register(s1);

float4 main(PixelShaderInput IN) : SV_Target
{
    // Sample textures
    float2 uv           = IN.Texture;
    uv.y                = 1.0f - uv.y;
    
    // Calculate the TBN matrix
    float4 normal       = Materials[Model.NormalTextureIndex].Sample(PointSampler, uv);
    float3 bitan        = cross(IN.Normal, IN.Tangent);
    float3x3 TBN        = float3x3(IN.Tangent, bitan, IN.Normal);

    // Setup surface
    Surface surface;
    surface.Positon     = IN.WorldPosition;
    surface.Albedo      = Materials[Model.AlbedoTextureIndex].Sample(LinearSampler, uv);;
    surface.Normal      = float4(normalize(mul(normal.xyz, TBN)), 0.0f);
    surface.Metalness   = Materials[Model.MetalnessTextureIndex].Sample(PointSampler, uv);;

    surface.FinalColor = CalculateAmbient(surface);
    for (int i = 0; i < Scene.LightsNum; ++i)
    {
        surface.FinalColor += CalculateDiffuse(surface, Lights[i]);
    }
    
    return float4(surface.FinalColor.rgb, surface.Albedo.w);
}
