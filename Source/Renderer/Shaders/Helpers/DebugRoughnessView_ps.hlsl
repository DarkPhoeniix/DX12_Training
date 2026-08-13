
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
    
    float roughness = source.Sample(PointClampSampler, input.UV).a;
    float4 color = float4(roughness, roughness, roughness, 1.0f);
    
    return color;
}
