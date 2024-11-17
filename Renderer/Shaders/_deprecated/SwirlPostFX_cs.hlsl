
#include "PostFX_rootsig.hlsli"

RWTexture2D<float4> AlbedoTexture : register(u0);
SamplerState Sampler : register(s0);


float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(92., 80.))) +
		cos(dot(co.xy, float2(41., 62.))) * 5.1);
}

[RootSignature(PostFX_RootSig)]
[numthreads(2, 2, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    float2 size = float2(1280.0f, 720.0f);
    
    float radius = 300.0;
    float angle = 1.8;
    float2 center = size / 2.0f;

    float2 texSize = size;
    float2 uv = float2(DTid.xy) / size;

    float2 tc = uv * texSize;
    tc -= center;
    float dist = length(tc);
    if (dist < radius)
    {
        float percent = (radius - dist) / radius;
        float theta = percent * percent * angle * 8.0f;
        float s = sin(theta);
        float c = cos(theta);
        tc = float2(dot(tc, float2(c, -s)), dot(tc, float2(s, c)));
    }
    tc += center;
    float4 sourceColor = AlbedoTexture[(tc / texSize) * size];
    
    AlbedoTexture[DTid.xy] = sourceColor;
}
