
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
    
    //float2 uv = float2(DTid.xy) / size;
    //float2 rnd = float2(rand(uv.xy), rand(uv.yx));
    //float4 sourceColor = AlbedoTexture[(uv + rnd * 0.05f) * size];
    
    
    float2 center = float2(0.5f, 0.5f);
    float spiralStrength = 0.1f;
    float distanceThreshold = 1.0f;


    float2 uv = DTid.xy / size;
    float2 dir = uv - center;
    float l = length(dir);

    dir = dir / l;
    float angle = atan2(dir.y, dir.x);

    float remainder = frac(l / distanceThreshold);

    float preTransitionWidth = 0.25;
    float fac;

    if (remainder < .25)
    {
        fac = 1.0;
    }
    else if (remainder < 0.5)
    {
		// transition zone - go smoothly from previous zone to next.
        fac = 1 - 8 * (remainder - preTransitionWidth);
    }
    else if (remainder < 0.75)
    {
        fac = -1.0;
    }
    else
    {
		// transition zone - go smoothly from previous zone to next.
        fac = -(1 - 8 * (remainder - 0.75));
    }

    float newAng = angle + fac * spiralStrength * l;

    float xAmt = cos(newAng) * l;
    float yAmt = sin(newAng) * l;

    float2 newCoord = center + float2(xAmt, yAmt);

    float4 sourceColor = AlbedoTexture[newCoord * size];
    
    AlbedoTexture[DTid.xy] = sourceColor;
}
