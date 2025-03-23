//
// This code is based on the FXAA 3.11 implementation by James Stanard (Minigraph),
// licensed under the MIT License (MIT), and NVIDIA FXAA 3.11 by Timothy Lottes.
// 

#include "FXAA_rootsig.hlsli"

RWByteAddressBuffer IndirectParams  : register(u0);
RWByteAddressBuffer WorkQueue       : register(u1);
RWByteAddressBuffer WorkCounts      : register(u2);

[RootSignature(FXAA_RootSig)]
[numthreads(64, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint2 PixelCounts = WorkCounts.Load2(0);

    // Write out padding to the buffer
    uint PixelCountH = PixelCounts.x;
    uint PaddedCountH = (PixelCountH + 63) & ~63;
    if (GI + PixelCountH < PaddedCountH)
    {
        WorkQueue.Store((PixelCountH + GI) * 4, 0xffffffff);
    }

    // Write out padding to the buffer
    uint PixelCountV = PixelCounts.y;
    uint PaddedCountV = (PixelCountV + 63) & ~63;
    if (GI + PixelCountV < PaddedCountV)
    {
        WorkQueue.Store((LastQueueIndex - PixelCountV - GI) * 4, 0xffffffff);
    }

    DeviceMemoryBarrierWithGroupSync();

    if (GI == 0)
    {
        IndirectParams.Store(0, PaddedCountH >> 6);
        IndirectParams.Store(12, PaddedCountV >> 6);
        WorkCounts.Store2(0, 0);
    }
}
