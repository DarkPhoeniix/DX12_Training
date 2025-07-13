
#define FXAA_RootSig \
	"RootFlags(0), " \
    "RootConstants(b0, num32BitConstants = 7), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t1, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t2, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(UAV(u0, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(UAV(u1, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(UAV(u2, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(UAV(u3, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL), " \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)"

cbuffer CB0 : register(b0)
{
    float2 RcpTextureSize;
    float ContrastThreshold;
    float SubpixelRemoval;
    uint LastQueueIndex;
    uint2 StartPixel;
}
