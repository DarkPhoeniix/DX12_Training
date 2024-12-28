
#include "Common.hlsli"

float rgbToLuma(float3 rgb)
{
    //return sqrt(dot(rgb, float3(0.299f, 0.587f, 0.114f)));
    
    return rgb.y * (0.587 / 0.299) + rgb.x;
}

float4 TextureOffset(SamplerState s, Texture2D texture, float2 uv, int2 offset = int2(0, 0))
{
    return texture.SampleLevel(s, uv, 0, offset);
}
