
#include "PostFX_rootsig.hlsli"

RWTexture2D<float4> AlbedoTexture : register(u0);
SamplerState Sampler : register(s0);

float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(92., 80.))) + cos(dot(co.xy, float2(41., 62.))) * 5.1);
}

[RootSignature(PostFX_RootSig)]
[numthreads(2, 2, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = AlbedoTexture[DTid.xy];
    color = 1.0f - color;
    color.a = 1.0f;
    
    AlbedoTexture[DTid.xy] = color;
}
