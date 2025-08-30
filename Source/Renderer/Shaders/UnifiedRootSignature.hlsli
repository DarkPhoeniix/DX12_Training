
// b0 - Frame constants
// b1 - immediate constants (pass constants)
// s0 - point clamp sampler
// s1 - point wrap sampler
// s2 - point mirror sampler
// s3 - point border sampler
// s4 - linear clamp sampler
// s5 - linear wrap sampler
// s6 - linear mirror sampler
// s7 - linear border sampler
// s8 - comparison linear clamp sampler (shadows)

#define URootSignature \
	"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_ALL), " \
    "RootConstants(num32BitConstants = 16, b1, visibility = SHADER_VISIBILITY_ALL), " \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_POINT)," \
    "StaticSampler(s1," \
        "addressU = TEXTURE_ADDRESS_WRAP," \
        "addressV = TEXTURE_ADDRESS_WRAP," \
        "addressW = TEXTURE_ADDRESS_WRAP," \
        "filter = FILTER_MIN_MAG_MIP_POINT)," \
    "StaticSampler(s2," \
        "addressU = TEXTURE_ADDRESS_MIRROR," \
        "addressV = TEXTURE_ADDRESS_MIRROR," \
        "addressW = TEXTURE_ADDRESS_MIRROR," \
        "filter = FILTER_MIN_MAG_MIP_POINT)," \
    "StaticSampler(s3," \
        "addressU = TEXTURE_ADDRESS_BORDER," \
        "addressV = TEXTURE_ADDRESS_BORDER," \
        "addressW = TEXTURE_ADDRESS_BORDER," \
        "filter = FILTER_MIN_MAG_MIP_POINT)," \
    "StaticSampler(s4," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    "StaticSampler(s5," \
        "addressU = TEXTURE_ADDRESS_WRAP," \
        "addressV = TEXTURE_ADDRESS_WRAP," \
        "addressW = TEXTURE_ADDRESS_WRAP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    "StaticSampler(s6," \
        "addressU = TEXTURE_ADDRESS_MIRROR," \
        "addressV = TEXTURE_ADDRESS_MIRROR," \
        "addressW = TEXTURE_ADDRESS_MIRROR," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    "StaticSampler(s7," \
        "addressU = TEXTURE_ADDRESS_BORDER," \
        "addressV = TEXTURE_ADDRESS_BORDER," \
        "addressW = TEXTURE_ADDRESS_BORDER," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    "StaticSampler(s8," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_COMPARISON_MIN_MAG_MIP_LINEAR," \
        "comparisonFunc = COMPARISON_LESS)"
