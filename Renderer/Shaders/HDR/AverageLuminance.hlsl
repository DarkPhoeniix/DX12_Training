
#define AverageLuminance_RootSig \
    "RootFlags " \
	"( " \
		"DENY_VERTEX_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS | " \
		"DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
		"DENY_PIXEL_SHADER_ROOT_ACCESS " \
	"), " \
    "RootConstants(num32BitConstants = 5, b0, visibility = SHADER_VISIBILITY_ALL), " \
    "SRV(t0, visibility = SHADER_VISIBILITY_ALL)," \
    "UAV(u0, visibility = SHADER_VISIBILITY_ALL)," \
    "UAV(u1, visibility = SHADER_VISIBILITY_ALL)"

#define NUM_HISTOGRAM_BINS 256

cbuffer LuminanceHistogramParametersCB          : register(b0)
{
    uint  PixelCount                            : packoffset(c0.x);
    float MinLogLuminance                       : packoffset(c0.y);
    float LogLuminanceRange                     : packoffset(c0.z);
    float DeltaTime                             : packoffset(c0.w);
    
    float Adaptation                            : packoffset(c1.x);
};
StructuredBuffer<float>     PrevAverageLum      : register(t0);
RWStructuredBuffer<uint>    LuminanceHistogram  : register(u0);
RWStructuredBuffer<float>   LuminanceOutput     : register(u1);

groupshared float HistogramShared[NUM_HISTOGRAM_BINS];

[RootSignature(AverageLuminance_RootSig)]
[numthreads(NUM_HISTOGRAM_BINS, 1, 1)]
void main(uint3 localThreadIndex : SV_GroupThreadID)
{
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
        float weightedLogAverage = (HistogramShared[0] / max((float) PixelCount - countForThisBin, 1.0)) - 1.0;
        float weightedAverageLuminance = exp2(((weightedLogAverage / (NUM_HISTOGRAM_BINS - 2)) * LogLuminanceRange) + MinLogLuminance);
        float adaptedLuminance = PrevAverageLum[0] + (weightedAverageLuminance - PrevAverageLum[0]) * Adaptation;
        LuminanceOutput[0] = adaptedLuminance;
    }
}