// https://www.alextardif.com/HistogramLuminance.html

#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../ToneMapping.hlsli"

#define NUM_HISTOGRAM_BINS 256
#define THREADS_PER_DIMENSION 16
#define EPSILON 0.001f

struct PassCB
{
    float MinLogLuminance;
    float OneOverLogLuminanceRange;
    
    uint HDRTextureIndex;
    uint LuminanceHistogramBufferIndex;
};

ConstantBuffer<PassCB> PassConstants    : register(b1);

groupshared uint HistogramShared[NUM_HISTOGRAM_BINS];

uint HDRToHistogramBin(float3 hdrColor)
{
    float luminance = Luminance(hdrColor);
    
    if (luminance < EPSILON)
    {
        return 0;
    }
    
    float logLuminance = saturate((log2(luminance) - PassConstants.MinLogLuminance) * PassConstants.OneOverLogLuminanceRange);
    return (uint)(logLuminance * (NUM_HISTOGRAM_BINS - 2) + 1.0f);
}

[RootSignature(URootSignature)]
[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 threadId : SV_DispatchThreadID, uint3 localThreadId : SV_GroupThreadID)
{
    if (threadId.x >= FrameCB.WindowSize.x || threadId.y >= FrameCB.WindowSize.y)
    {
        return; // Out of bounds
    }
    
    Texture2D HDRTexture                        = ResourceDescriptorHeap[PassConstants.HDRTextureIndex];
    RWStructuredBuffer<uint> LuminanceHistogram = ResourceDescriptorHeap[PassConstants.LuminanceHistogramBufferIndex];
    
    HistogramShared[localThreadId.x + localThreadId.y * THREADS_PER_DIMENSION] = 0;
    
    GroupMemoryBarrierWithGroupSync();
    
    float3 hdrColor = HDRTexture.Load(uint3(threadId.xy, 0)).rgb;
    uint binIndex = HDRToHistogramBin(hdrColor);
    InterlockedAdd(HistogramShared[binIndex], 1);
    
    GroupMemoryBarrierWithGroupSync();
    
    InterlockedAdd(LuminanceHistogram[groupIndex], HistogramShared[groupIndex]);
}