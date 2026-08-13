
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"

struct PSInput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PassConstants
{
    uint AlbedoMetallicTextureIndex;
};

URootConstants(PassConstants, PassCB);

float4 main(PSInput input) : SV_Target0
{
    Texture2D source = ResourceDescriptorHeap[PassCB.AlbedoMetallicTextureIndex];
    
    float metallic = source.Sample(PointClampSampler, input.UV).a;
    float4 color = float4(metallic, metallic, metallic, 1.0f);
    
    return color;
}
