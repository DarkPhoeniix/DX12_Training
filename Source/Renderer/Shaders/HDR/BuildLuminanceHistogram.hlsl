// https://www.alextardif.com/HistogramLuminance.html

#include "ToneMapping.hlsli"

#define BuildLuminanceHistogram_RootSig \
    "RootFlags " \
	"( " \
		"DENY_VERTEX_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS | " \
		"DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
		"DENY_PIXEL_SHADER_ROOT_ACCESS " \
	"), " \
    "RootConstants(num32BitConstants = 4, b0, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_ALL)," \
    "UAV(u0, visibility = SHADER_VISIBILITY_ALL)"

#define NUM_HISTOGRAM_BINS 256
#define THREADS_PER_DIMENSION 16
#define EPSILON 0.001f

cbuffer LuminanceHistogramParametersCB      : register(b0)
{
    uint  InputWidth;
    uint  InputHeight;
    float MinLogLuminance;
    float OneOverLogLuminanceRange;
};
Texture2D                HDRTexture         : register(t0);
RWStructuredBuffer<uint> LuminanceHistogram : register(u0);

groupshared uint HistogramShared[NUM_HISTOGRAM_BINS];

uint HDRToHistogramBin(float3 hdrColor)
{
    float luminance = Luminance(hdrColor);
    
    if (luminance < EPSILON)
    {
        return 0;
    }
    
    float logLuminance = saturate((log2(luminance) - MinLogLuminance) * OneOverLogLuminanceRange);
    return (uint)(logLuminance * (NUM_HISTOGRAM_BINS - 2) + 1.0f);
}

[RootSignature(BuildLuminanceHistogram_RootSig)]
[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 threadId : SV_DispatchThreadID, uint3 localThreadId : SV_GroupThreadID)
{
    HistogramShared[localThreadId.x + localThreadId.y * THREADS_PER_DIMENSION] = 0;
    
    GroupMemoryBarrierWithGroupSync();
    
    if (threadId.x < InputWidth && threadId.y < InputHeight)
    {
        float3 hdrColor = HDRTexture.Load(uint3(threadId.xy, 0)).rgb;
        uint binIndex = HDRToHistogramBin(hdrColor);
        InterlockedAdd(HistogramShared[binIndex], 1);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    InterlockedAdd(LuminanceHistogram[groupIndex], HistogramShared[groupIndex]);
}