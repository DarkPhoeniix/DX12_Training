//
// This code is based on the FXAA 3.11 implementation by James Stanard (Minigraph),
// licensed under the MIT License (MIT), and NVIDIA FXAA 3.11 by Timothy Lottes.
// 

#include "FXAA_rootsig.hlsli"
#include "../PixelPacking.hlsli"

Texture2D<float> Luma           : register(t0);
ByteAddressBuffer WorkQueue     : register(t1);
Buffer<uint> ColorQueue         : register(t2);

RWTexture2D<float3> DstColor    : register(u0);

SamplerState LinearSampler      : register(s0);

// Note that the number of samples in each direction is one less than the number of sample distances.  The last
// is the maximum distance that should be used, but whether that sample is "good" or "bad" doesn't affect the result,
// so we don't need to load it.
//#define FXAA_EXTREME_QUALITY

#ifdef FXAA_EXTREME_QUALITY
#define NUM_SAMPLES 11
    static const float s_SampleDistances[12] =	// FXAA_QUALITY__PRESET == 39
    {
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.5f, 8.5f, 10.5f, 12.5f, 14.5f, 18.5f, 36.5f 
    };
#else
#define NUM_SAMPLES 7
static const float s_SampleDistances[8] = // FXAA_QUALITY__PRESET == 25
{
    1.0f, 2.5f, 4.5f, 6.5f, 8.5f, 10.5f, 14.5f, 22.5f
};
#endif

[RootSignature(FXAA_RootSig)]
[numthreads(64, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
#ifdef VERTICAL_ORIENTATION
    uint ItemIdx = LastQueueIndex - DTid.x;
#else
    uint ItemIdx = DTid.x;
#endif
    uint WorkHeader = WorkQueue.Load(ItemIdx * 4);
    uint2 ST = StartPixel + (uint2(WorkHeader >> 8, WorkHeader >> 20) & 0xFFF);
    uint GradientDir = WorkHeader & 1; // Determines which side of the pixel has the highest contrast
    float Subpix = (WorkHeader & 0xFE) / 254.0f * 0.5f; // 7-bits to encode [0, 0.5]

#ifdef VERTICAL_ORIENTATION
    float NextLuma = Luma[ST + int2(GradientDir * 2 - 1, 0)];
    float2 StartUV = (ST + float2(GradientDir, 0.5f)) * RcpTextureSize;
#else
    float NextLuma = Luma[ST + int2(0, GradientDir * 2 - 1)];
    float2 StartUV = (ST + float2(0.5, GradientDir)) * RcpTextureSize;
#endif
    float ThisLuma = Luma[ST];
    float CenterLuma = (NextLuma + ThisLuma) * 0.5f; // Halfway between this and next; center of the contrasting edge
    float GradientSgn = sign(NextLuma - ThisLuma); // Going down in brightness or up?
    float GradientMag = abs(NextLuma - ThisLuma) * 0.25f; // How much contrast?  When can we stop looking?

    float NegDist = s_SampleDistances[NUM_SAMPLES];
    float PosDist = s_SampleDistances[NUM_SAMPLES];
    bool NegGood = false;
    bool PosGood = false;

    for (uint iter = 0; iter < NUM_SAMPLES; ++iter)
    {
        const float Distance = s_SampleDistances[iter];

#ifdef VERTICAL_ORIENTATION
        float2 NegUV = StartUV - float2(0, RcpTextureSize.y) * Distance;
        float2 PosUV = StartUV + float2(0, RcpTextureSize.y) * Distance;
#else
        float2 NegUV = StartUV - float2(RcpTextureSize.x, 0) * Distance;
        float2 PosUV = StartUV + float2(RcpTextureSize.x, 0) * Distance;
#endif

        // Check for a negative endpoint
        float NegGrad = Luma.SampleLevel(LinearSampler, NegUV, 0) - CenterLuma;
        if (abs(NegGrad) >= GradientMag && Distance < NegDist)
        {
            NegDist = Distance;
            NegGood = sign(NegGrad) == GradientSgn;
        }

        // Check for a positive endpoint
        float PosGrad = Luma.SampleLevel(LinearSampler, PosUV, 0) - CenterLuma;
        if (abs(PosGrad) >= GradientMag && Distance < PosDist)
        {
            PosDist = Distance;
            PosGood = sign(PosGrad) == GradientSgn;
        }
    }

    // Ranges from 0.0 to 0.5
    float PixelShift = 0.5f - min(NegDist, PosDist) / (PosDist + NegDist);
    bool GoodSpan = NegDist < PosDist ? NegGood : PosGood;
    PixelShift = max(Subpix, GoodSpan ? PixelShift : 0.0f);

    if (PixelShift > 0.01f)
    {
#ifdef FXAA_DEBUG
        DstColor[ST] = float3(2.0 * PixelShift, 1.0 - 2.0 * PixelShift, 0);
#else
        DstColor[ST] = lerp(DstColor[ST], Unpack_R11G11B10_FLOAT(ColorQueue[ItemIdx]), PixelShift);
#endif
    }
}
