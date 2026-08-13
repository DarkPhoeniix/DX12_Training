
#include "../UnifiedRootSignature.hlsli"

#define NUM_THREADS 16

struct RootConstants
{
    uint AmbientOcculusionTextureIndex;
    uint TargetTextureIndex;
};

URootConstants(RootConstants, RootCB);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
[RootSignature(URootSignature)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Texture2D<float> AOTexture = ResourceDescriptorHeap[RootCB.AmbientOcculusionTextureIndex];
    RWTexture2D<float4> Target = ResourceDescriptorHeap[RootCB.TargetTextureIndex];
    
    uint2 pixel = DTid.xy;
    
    uint ScreenWidth, ScreenHeight;
    Target.GetDimensions(ScreenWidth, ScreenHeight);
    
    if (pixel.x >= ScreenWidth || pixel.y >= ScreenHeight)
    {
        return;
    }
    
    float3 color = Target[pixel].rgb;
    color *= AOTexture[pixel];
    
    Target[pixel] = float4(color, Target[pixel].a);
}