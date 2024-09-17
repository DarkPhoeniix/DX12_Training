
// The Vertex Shader (VS) stage is responsible for transforming the vertex data 
// from object-space into clip-space, performing (skeletal) animation or computing 
// per-vertex lighting.

#include "Mesh_rootsig.hlsli"

#include "Common.hlsli"

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
    float2 Texture  : TEXCOORD;
    float3 Tangent  : TANGENT;
    
    uint sv_instance : SV_InstanceID;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
    float2 Texture  : TEXCOORD;
    float3 Tangent  : TANGENT;
};

[RootSignature(Mesh_RootSig)]
VSOutput main(VSInput IN)
{
    VSOutput output;
    
    float4 worldPosition = mul(float4(IN.Position, 1.0f), Model.Transform);

    output.Position = mul(worldPosition, Scene.ViewProjection);
    output.Normal   = mul(float4(IN.Normal, 0.0f), Model.Transform).xyz;
    output.Color    = IN.Color;
    output.Texture  = IN.Texture;
    output.Tangent  = IN.Tangent;

    return output;
}
