
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"

#define NUM_HISTOGRAM_BINS 256

struct PassCB
{
    uint PixelCount;
    float MinLogLuminance;
    float LogLuminanceRange;
    
    uint PrevLuminanceIndex;
    uint LuminanceHistogramIndex;
    uint OutputLuminanceIndex;
};

ConstantBuffer<PassCB> PassConstants : register(b1);

groupshared float HistogramShared[NUM_HISTOGRAM_BINS];

[RootSignature(URootSignature)]
[numthreads(NUM_HISTOGRAM_BINS, 1, 1)]
void main(uint3 localThreadIndex : SV_GroupThreadID)
{
    StructuredBuffer<float> PrevAverageLum      = ResourceDescriptorHeap[PassConstants.PrevLuminanceIndex];
    RWStructuredBuffer<uint> LuminanceHistogram = ResourceDescriptorHeap[PassConstants.LuminanceHistogramIndex];
    RWStructuredBuffer<float> LuminanceOutput   = ResourceDescriptorHeap[PassConstants.OutputLuminanceIndex];
    
    uint threadIndex = localThreadIndex.x;
    float countForThisBin = (float) LuminanceHistogram.Load(threadIndex);
    HistogramShared[threadIndex] = countForThisBin * (float) threadIndex;
    
    GroupMemoryBarrierWithGroupSync();
    
    LuminanceHistogram[threadIndex] = 0;
    
    [unroll]
    for (uint histogramSampleIndex = (NUM_HISTOGRAM_BINS >> 1); histogramSampleIndex > 0; histogramSampleIndex >>= 1)
    {
        if (threadIndex < histogramSampleIndex)
        {
            HistogramShared[threadIndex] += HistogramShared[threadIndex + histogramSampleIndex];
        }
        
        GroupMemoryBarrierWithGroupSync();
    }
    
    if (threadIndex == 0)
    {
        float adaptation = min(FrameCB.DeltaTime * 2.5f, 1.0f);
        
        float weightedLogAverage = (HistogramShared[0] / max((float) PassConstants.PixelCount - countForThisBin, 1.0)) - 1.0;
        float weightedAverageLuminance = exp2(((weightedLogAverage / (NUM_HISTOGRAM_BINS - 2)) * PassConstants.LogLuminanceRange) + PassConstants.MinLogLuminance);
        float adaptedLuminance = PrevAverageLum[0] + (weightedAverageLuminance - PrevAverageLum[0]) * adaptation;
        LuminanceOutput[0] = adaptedLuminance;
    }
}