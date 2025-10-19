
#include "../CommonResources.hlsli"

struct PSInput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PassConstants
{
    uint SSAOTextureIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

float4 main(PSInput input) : SV_Target0
{
    Texture2D source = ResourceDescriptorHeap[PassCB.SSAOTextureIndex];
    
    float3 ssao = source.Sample(PointClampSampler, input.UV).rgb;
    float4 color = float4(ssao, 1.0f);
    
    return color;
}
