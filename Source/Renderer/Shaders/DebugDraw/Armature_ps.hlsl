
#include "../UnifiedRootSignature.hlsli"

struct Pixelinput
{
    float4 Position : SV_Position;
    float2 Color    : COLOR;
};

float4 main(Pixelinput input) : SV_Target
{
    return float4(input.Color, 0.0f, 1.0f);
}
