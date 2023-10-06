
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
	"RootConstants(num32BitConstants=16, b1, visibility=SHADER_VISIBILITY_ALL), " \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    "StaticSampler(s1," \
        "addressU = TEXTURE_ADDRESS_WRAP," \
        "addressV = TEXTURE_ADDRESS_WRAP," \
        "addressW = TEXTURE_ADDRESS_WRAP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
	"StaticSampler(s2," \
		"addressU = TEXTURE_ADDRESS_WRAP," \
		"addressV = TEXTURE_ADDRESS_WRAP," \
		"addressW = TEXTURE_ADDRESS_WRAP," \
		"filter = FILTER_ANISOTROPIC,"\
		"maxAnisotropy=2)," \
	"StaticSampler(s3," \
		"addressU = TEXTURE_ADDRESS_WRAP," \
		"addressV = TEXTURE_ADDRESS_WRAP," \
		"addressW = TEXTURE_ADDRESS_WRAP," \
		"filter = FILTER_ANISOTROPIC,"\
		"maxAnisotropy=4)" 

struct ModelViewProjection
{
    matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

//struct ModelViewProjection1
//{
//    matrix MVP;
//};

//ConstantBuffer<ModelViewProjection1> ModelViewProjectionCB1 : register(b1);

struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

[RootSignature(Sprite_RootSig)]
VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.Color, 1.0f);

    return OUT;
}
