
#define Sprite_RootSig \
	"RootFlags " \
	"( " \
		"ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | " \
		"DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS " \
	"), " \
    "RootConstants(num32BitConstants=16, b0, visibility=SHADER_VISIBILITY_ALL), " \
    "CBV(b1, visibility=SHADER_VISIBILITY_PIXEL), " \
	"SRV(t0, visibility=SHADER_VISIBILITY_ALL), " \
    "SRV(t1,visibility=SHADER_VISIBILITY_PIXEL)," \
    "SRV(t2,visibility=SHADER_VISIBILITY_PIXEL)," \
    "DescriptorTable(SRV(t3),visibility=SHADER_VISIBILITY_PIXEL)," \
    "DescriptorTable(SRV(t4),visibility=SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_MIRROR," \
        "addressV = TEXTURE_ADDRESS_MIRROR," \
        "addressW = TEXTURE_ADDRESS_MIRROR," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    "StaticSampler(s1," \
        "addressU = TEXTURE_ADDRESS_WRAP," \
        "addressV = TEXTURE_ADDRESS_WRAP," \
        "addressW = TEXTURE_ADDRESS_WRAP," \
        "filter = FILTER_MIN_MAG_MIP_POINT)," \