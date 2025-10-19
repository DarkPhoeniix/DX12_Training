
#include "../CommonResources.hlsli"

struct PSInput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PassConstants
{
    uint EmissionTextureIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

float4 main(PSInput input) : SV_Target0
{
    Texture2D source = ResourceDescriptorHeap[PassCB.EmissionTextureIndex];
    
    float3 color = source.Sample(PointClampSampler, input.UV).rgb;
    float intensity = source.Sample(PointClampSampler, input.UV).a;
    float4 output = float4(color * intensity, 1.0f);
    
    return output;
}
