
#define DeferredShading_RootSig \
	"RootFlags " \
	"( " \
		"ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | " \
		"DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS " \
	"), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_ALL), " \
    "CBV(b1, visibility = SHADER_VISIBILITY_ALL), " \
    "SRV(t0, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t1), visibility=SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(SRV(t2), visibility=SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(SRV(t3), visibility=SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(SRV(t4, numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(UAV(u0), visibility=SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_WRAP," \
        "addressV = TEXTURE_ADDRESS_WRAP," \
        "addressW = TEXTURE_ADDRESS_WRAP," \
        "filter = FILTER_MIN_MAG_MIP_POINT)"
