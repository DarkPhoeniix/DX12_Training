
#include "../UnifiedRootSignature.hlsli"
#include "../ToneMapping.hlsli"

struct PassConstants
{
    float MiddleGrey;
    float LumWhiteSqr;
    float Gamma;
    
    uint HDRTextureIndex;
    uint AverageLuminanceBufferIndex;
    uint TargetTextureIndex;
};
URootConstants(PassConstants, PassCB);

[RootSignature(URootSignature)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Texture2D HDRTexture                        = ResourceDescriptorHeap[PassCB.HDRTextureIndex];
    StructuredBuffer<float> AvgLuminanceBuffer  = ResourceDescriptorHeap[PassCB.AverageLuminanceBufferIndex];
    RWTexture2D<float4> TargetTetxure           = ResourceDescriptorHeap[PassCB.TargetTextureIndex];
    
    float3 color = HDRTexture.Load(uint3(DTid.xy, 0)).rgb;
    color = ExtendedReinhardToneMapping(color, AvgLuminanceBuffer[0], PassCB.MiddleGrey, PassCB.LumWhiteSqr, PassCB.Gamma);
    
    TargetTetxure[DTid.xy] = float4(color, 1.0f);
}
