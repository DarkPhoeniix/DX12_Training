//
// This code is based on the FXAA 3.11 implementation by James Stanard (Minigraph),
// licensed under the MIT License (MIT), and NVIDIA FXAA 3.11 by Timothy Lottes.
// 

#include "../UnifiedRootSignature.hlsli"
#include "../PixelPacking.hlsli"

struct PassConstants
{
    float ContrastThreshold;
    float SubpixelRemoval;
    uint2 StartPixel;
    uint LastQueueIndex;
    
    uint InputTextureIndex;
    uint WorkCountBufferIndex;
    uint WorkQueueBufferIndex;
    uint ColorQueueBufferIndex;
    uint LumaTextureIndex;
};

URootConstants(PassConstants, PassCB);

#define BOUNDARY_SIZE 1
#define ROW_WIDTH (8 + BOUNDARY_SIZE * 2)
groupshared float gs_LumaCache[ROW_WIDTH * ROW_WIDTH];

//
// Helper functions
//
float RGBToLogLuminance(float3 LinearRGB)
{
    float Luma = dot(LinearRGB, float3(0.212671f, 0.715160f, 0.072169f));
    return log2(1.0f + Luma * 15.0f) / 4.0f;
}

[RootSignature(URootSignature)]
[numthreads(8, 8, 1)]
void main(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    Texture2D<float3> Color         = ResourceDescriptorHeap[PassCB.InputTextureIndex];

    RWByteAddressBuffer WorkCount   = ResourceDescriptorHeap[PassCB.WorkCountBufferIndex];
    RWByteAddressBuffer WorkQueue   = ResourceDescriptorHeap[PassCB.WorkQueueBufferIndex];
    RWBuffer<uint> ColorQueue       = ResourceDescriptorHeap[PassCB.ColorQueueBufferIndex];
    RWTexture2D<float> Luma         = ResourceDescriptorHeap[PassCB.LumaTextureIndex];
    
    uint2 PixelCoord = DTid.xy + PassCB.StartPixel;

    // Because we can't use Gather() on RGB, we make each thread read two pixels (but only those needed).
    if (GI < ROW_WIDTH * ROW_WIDTH / 2.0f)
    {
        uint LdsCoord = GI;
        int2 UavCoord = PassCB.StartPixel + uint2(GI % ROW_WIDTH, GI / ROW_WIDTH) + Gid.xy * 8 - BOUNDARY_SIZE;
        float Luma1 = RGBToLogLuminance(Color[UavCoord]);
        Luma[UavCoord] = Luma1;
        gs_LumaCache[LdsCoord] = Luma1;

        LdsCoord += ROW_WIDTH * ROW_WIDTH / 2;
        UavCoord += int2(0, ROW_WIDTH / 2);
        float Luma2 = RGBToLogLuminance(Color[UavCoord]);
        Luma[UavCoord] = Luma2;
        gs_LumaCache[LdsCoord] = Luma2;
    }
    GroupMemoryBarrierWithGroupSync();

    uint CenterIdx = (GTid.x + BOUNDARY_SIZE) + (GTid.y + BOUNDARY_SIZE) * ROW_WIDTH;

    // Load the ordinal and center luminances
    float lumaN = gs_LumaCache[CenterIdx - ROW_WIDTH];
    float lumaW = gs_LumaCache[CenterIdx - 1];
    float lumaM = gs_LumaCache[CenterIdx];
    float lumaE = gs_LumaCache[CenterIdx + 1];
    float lumaS = gs_LumaCache[CenterIdx + ROW_WIDTH];

    // Contrast threshold test
    float rangeMax = max(max(lumaN, lumaW), max(lumaE, max(lumaS, lumaM)));
    float rangeMin = min(min(lumaN, lumaW), min(lumaE, min(lumaS, lumaM)));
    float range = rangeMax - rangeMin;
    if (range < PassCB.ContrastThreshold)
        return;

    // Load the corner luminances
    float lumaNW = gs_LumaCache[CenterIdx - ROW_WIDTH - 1];
    float lumaNE = gs_LumaCache[CenterIdx - ROW_WIDTH + 1];
    float lumaSW = gs_LumaCache[CenterIdx + ROW_WIDTH - 1];
    float lumaSE = gs_LumaCache[CenterIdx + ROW_WIDTH + 1];

    // Pre-sum a few terms so the results can be reused
    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;
    float lumaNWSW = lumaNW + lumaSW;
    float lumaNESE = lumaNE + lumaSE;
    float lumaSWSE = lumaSW + lumaSE;
    float lumaNWNE = lumaNW + lumaNE;

    // Compute horizontal and vertical contrast; see which is bigger
    float edgeHorz = abs(lumaNWSW - 2.0f * lumaW) + abs(lumaNS - 2.0f * lumaM) * 2.0f + abs(lumaNESE - 2.0f * lumaE);
    float edgeVert = abs(lumaSWSE - 2.0f * lumaS) + abs(lumaWE - 2.0f * lumaM) * 2.0f + abs(lumaNWNE - 2.0f * lumaN);

    // Also compute local contrast in the 3x3 region.  This can identify standalone pixels that alias.
    float avgNeighborLuma = ((lumaNS + lumaWE) * 2.0f + lumaNWSW + lumaNESE) / 12.0f;
    float subpixelShift = saturate(pow(smoothstep(0, 1, abs(avgNeighborLuma - lumaM) / range), 2) * PassCB.SubpixelRemoval * 2);

    float NegGrad = (edgeHorz >= edgeVert ? lumaN : lumaW) - lumaM;
    float PosGrad = (edgeHorz >= edgeVert ? lumaS : lumaE) - lumaM;
    uint GradientDir = abs(PosGrad) >= abs(NegGrad) ? 1 : 0;
    uint Subpix = uint(subpixelShift * 254.0f) & 0xFE;

    // Packet header: [ 12 bits Y | 12 bits X | 7 bit Subpix | 1 bit dir(Grad) ]
    uint WorkHeader = DTid.y << 20 | DTid.x << 8 | Subpix | GradientDir;

    if (edgeHorz > edgeVert)
    {
        uint WorkIdx;
        WorkCount.InterlockedAdd(0, 1, WorkIdx);
        WorkQueue.Store(WorkIdx * 4, WorkHeader);
        ColorQueue[WorkIdx] = Pack_R11G11B10_FLOAT(Color[float2(PixelCoord + uint2(0, 2 * GradientDir - 1))]);
    }
    else
    {
        uint WorkIdx;
        WorkCount.InterlockedAdd(4, 1, WorkIdx);
        WorkIdx = PassCB.LastQueueIndex - WorkIdx;
        WorkQueue.Store(WorkIdx * 4, WorkHeader);
        ColorQueue[WorkIdx] = Pack_R11G11B10_FLOAT(Color[float2(PixelCoord + uint2(2 * GradientDir - 1, 0))]);
    }
}