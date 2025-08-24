
#include "../UnifiedRootSignature.hlsli"

struct PassConstants
{
    float3 Position;
    float Radius;
    float4 Color;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

float4 main() : SV_TARGET
{
    return PassCB.Color;
}