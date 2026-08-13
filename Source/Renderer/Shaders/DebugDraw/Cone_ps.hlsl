
#include "../UnifiedRootSignature.hlsli"

struct PassConstants
{
    float3 Position;
    float Radius;
    float3 Direction;
    float Height;
    float4 Color;
};

URootConstants(PassConstants, PassCB);

float4 main() : SV_TARGET
{
    return PassCB.Color;
}