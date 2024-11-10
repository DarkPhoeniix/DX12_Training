
#include "GPass_rootsig.hlsli"

#include "Common.hlsli"

struct VSInput
{
    float3 Position         : POSITION;
    float3 Normal           : NORMAL;
    float4 Color            : COLOR;
    float2 Texture          : TEXCOORD;
    float3 Tangent          : TANGENT;
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

[RootSignature(GPass_RootSig)]
VSOutput main(VSInput IN)
{
    float4 worldPosition    = mul(float4(IN.Position, 1.0f), Model.Transform);
    
    VSOutput output;
    output.WorldPosition    = worldPosition;
    output.Position         = mul(worldPosition, Scene.ViewProjection);
    output.Normal           = normalize(mul(IN.Normal, (float3x3)Model.Transform));
    output.Color            = IN.Color;
    output.Texture          = IN.Texture;
    output.Tangent          = normalize(mul(IN.Tangent, (float3x3)Model.Transform));

    return output;
}
