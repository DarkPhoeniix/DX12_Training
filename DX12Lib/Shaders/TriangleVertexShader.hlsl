
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
	"SRV(t0, visibility=SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t1),visibility=SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_MIRROR," \
        "addressV = TEXTURE_ADDRESS_MIRROR," \
        "addressW = TEXTURE_ADDRESS_MIRROR," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \

struct ConstantsDesc
{
    row_major float4x4 ViewProj;
};

struct ModelDesc
{
    row_major matrix Model;
};

ConstantBuffer<ConstantsDesc> Constants : register(b0);
StructuredBuffer<ModelDesc> ModelSRV_CB : register(t0);

struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 Texture : TEXCOORD;
    
    uint sv_instance : SV_InstanceID;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 Normal : Normal;
    float4 Color : COLOR;
    float2 Texture : TEXCOORD;
};

[RootSignature(Sprite_RootSig)]
VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;

    float4 worldPosition = mul(float4(IN.Position, 1.0f), ModelSRV_CB[IN.sv_instance].Model);
    OUT.Position = mul(worldPosition, Constants.ViewProj);
    OUT.Normal = mul(float4(IN.Normal, 0.0f), ModelSRV_CB[IN.sv_instance].Model).xyz;
    OUT.Color = IN.Color;
    OUT.Texture = IN.Texture;

    return OUT;
}
