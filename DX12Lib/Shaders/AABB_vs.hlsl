
#define AABB_RootSig \
	"RootFlags " \
	"( " \
		"DENY_VERTEX_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS " \
	"), " \
	"RootConstants(num32BitConstants=8, b0, visibility=SHADER_VISIBILITY_ALL), " \
	"RootConstants(num32BitConstants=16, b1 , visibility=SHADER_VISIBILITY_ALL), " \
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

struct VertexInput
{
    uint primitive : SV_InstanceID;
};

struct GeometryInput
{
    uint primitive : INDEX;
};

[RootSignature(AABB_RootSig)]
GeometryInput main(VertexInput input)
{
    GeometryInput output = (GeometryInput) 0;
	
    output.primitive = input.primitive;
	
    return output;
}