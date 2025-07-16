
#define SSAOApply_RootSig \
	"RootFlags(0), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(UAV(u0), visibility = SHADER_VISIBILITY_ALL)"

#include "../CommonResources.hlsli"
#include "../DepthFuncs.hlsli"

#define NUM_THREADS 16

Texture2D<float> AOTexture      : register(t0);
RWTexture2D<float4> Target      : register(u0);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
[RootSignature(SSAOApply_RootSig)]
void main(uint3 DTid : SV_DispatchThreadID)
{
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