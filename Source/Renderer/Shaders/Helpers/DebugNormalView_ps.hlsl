
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"

struct PSInput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PassConstants
{
    uint NormalRoughnessTextureIndex;
};

URootConstants(PassConstants, PassCB);

float4 main(PSInput input) : SV_Target0
{
    Texture2D source = ResourceDescriptorHeap[PassCB.NormalRoughnessTextureIndex];
    
    float4 normal = float4(source.Sample(PointClampSampler, input.UV).xyz, 1.0f);
    
    return normal;
}
