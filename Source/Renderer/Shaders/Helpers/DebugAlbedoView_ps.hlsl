
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

ConstantBuffer<PassConstants> PassCB : register(b1);

float4 main(PSInput input) : SV_Target0
{
    Texture2D source = ResourceDescriptorHeap[PassCB.AlbedoMetallicTextureIndex];
    
    float4 color = float4(source.Sample(PointClampSampler, input.UV).rgb, 1.0f);
    
    return color;
}
