
#define ToneMapping_RootSig \
    "RootFlags " \
	"( " \
		"DENY_VERTEX_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS | " \
		"DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
		"DENY_PIXEL_SHADER_ROOT_ACCESS " \
	"), " \
    "RootConstants(num32BitConstants = 2, b0, visibility = SHADER_VISIBILITY_ALL), " \
    "SRV(t0, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t1), visibility = SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(UAV(u0), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_WRAP," \
        "addressV = TEXTURE_ADDRESS_WRAP," \
        "addressW = TEXTURE_ADDRESS_WRAP," \
        "filter = FILTER_MIN_MAG_MIP_POINT)"

#include "ToneMapping.hlsli"

cbuffer FinalPassConstants              : register(b0)
{
    float MiddleGrey    : packoffset(c0);
    float LumWhiteSqr   : packoffset(c0.y);
}
StructuredBuffer<float> AverageLum      : register(t0);
Texture2D HDRTexture                    : register(t1);
RWTexture2D<float4> OutputTexture       : register(u0);

[RootSignature(ToneMapping_RootSig)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float3 color = HDRTexture.Load(uint3(DTid.xy, 0)).rgb;
    color = ExtendedReinhardToneMapping(color, AverageLum[0], MiddleGrey, LumWhiteSqr);
    
    OutputTexture[DTid.xy] = float4(color, 1.0f);
}
