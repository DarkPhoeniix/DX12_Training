
// The Vertex Shader (VS) stage is responsible for transforming the vertex data 
// from object-space into clip-space, performing (skeletal) animation or computing 
// per-vertex lighting.

#include "Mesh_rootsig.hlsli"

struct ConstantsDesc
{
    row_major float4x4 ViewProj;
};

struct ModelDesc
{
    row_major matrix Transform;
};

ConstantBuffer<ConstantsDesc> Constants : register(b0);
StructuredBuffer<ModelDesc> Model : register(t0);

struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 Texture : TEXCOORD;
    float3 Tangent : TANGENT;
    
    uint sv_instance : SV_InstanceID;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 Texture : TEXCOORD;
    float3 Tangent : TANGENT;
};

[RootSignature(Sprite_RootSig)]
VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    
    row_major matrix modelTransform = Model[IN.sv_instance].Transform;
    float4 worldPosition = mul(float4(IN.Position, 1.0f), modelTransform);

    OUT.Position = mul(worldPosition, Constants.ViewProj);
    OUT.Normal = mul(float4(IN.Normal, 0.0f), modelTransform).xyz;
    OUT.Color = IN.Color;
    OUT.Texture = IN.Texture;
    OUT.Tangent = IN.Tangent;

    return OUT;
}
