
#include "../UnifiedRootSignature.hlsli"

struct PixelInput
{
    float4 Position : SV_POSITION;
};

struct PassConstants
{
    float3 BoxMin;
    float3 BoxMax;
    float4 Color;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

float4 main(PixelInput input) : SV_Target
{
    return PassCB.Color;
}
