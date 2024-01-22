
// The Vertex Shader (VS) stage is responsible for transforming the vertex data 
// from object-space into clip-space, performing (skeletal) animation or computing 
// per-vertex lighting.

#define Sprite_RootSig \
	"RootFlags " \
	"( " \
		"ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | " \
		"DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS " \
	"), " \
    "RootConstants(num32BitConstants=16, b0, visibility=SHADER_VISIBILITY_ALL), " \
    "CBV(b1, visibility=SHADER_VISIBILITY_ALL), " \
	"SRV(t1, visibility=SHADER_VISIBILITY_ALL) "

struct Constants
{
    row_major float4x4 ViewProj;
};

struct ModelViewProjection
{
    row_major matrix Model;
};

ConstantBuffer<Constants> Globals : register(b0);
StructuredBuffer<ModelViewProjection> ModelViewProjectionCB : register(t1);

struct VertexPosColor
{
    float3 PositionL : POSITION;
    float3 Normal : NORMAL;
    float3 Color : COLOR;
    
    uint sv_instance : SV_InstanceID;
};

struct VertexShaderOutput
{
    float4 PositionH : SV_Position;
    float4 Normal : Normal;
    float4 Color : COLOR;
};

[RootSignature(Sprite_RootSig)]
VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;

    float4 posW = mul(float4(IN.PositionL, 1.0f), ModelViewProjectionCB[IN.sv_instance].Model);
    
    OUT.PositionH = mul(posW, Globals.ViewProj);
    OUT.Normal = float4(IN.Normal, 0.0f);
    OUT.Color = float4(IN.Color, 1.0f);

    return OUT;
}
