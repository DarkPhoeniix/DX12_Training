
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../CommonConstants.hlsli"

#include "../DepthFuncs.hlsli"

struct PassConstants
{
    uint DepthTextureIndex;
    uint SkyboxTextureIndex;
    uint TargetTextureIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.x, v.z), asin(-v.y));
    uv *= float2(k_1_PI_2, k_1_PI);
    uv += 0.5f;
    return uv;
}

SamplerState LinearSampler : register(s0);

[RootSignature(URootSignature)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x > FrameCB.WindowSize.x || DTid.y > FrameCB.WindowSize.y)
    {
        return;
    }
    
    Texture2D<float4> DepthTexture      = ResourceDescriptorHeap[PassCB.DepthTextureIndex];
    Texture2D<float4> SkyboxTexture     = ResourceDescriptorHeap[PassCB.SkyboxTextureIndex];
    RWTexture2D<float4> TargetTexture   = ResourceDescriptorHeap[PassCB.TargetTextureIndex];

    float depth = DepthTexture.Load(uint3(DTid.xy, 0)).r;
    
    if (depth != 1.0f)
    {
        return;
    }
    
    float4 pos = ReconstructPosW(depth, DTid.xy, FrameCB.WindowSize, FrameCB.InvProjection, FrameCB.InvView);
    float4 dir = normalize(pos - FrameCB.EyePosition);
    uint x, y, z;
    SkyboxTexture.GetDimensions(0, x, y, z);
    float2 skyboxTexel = SampleSphericalMap(dir.xyz);
    //skyboxTexel *= float2(x - 1, y - 1);
    float4 color = SkyboxTexture.SampleLevel(LinearSampler, skyboxTexel, 0);
    
    TargetTexture[DTid.xy] = color;
}
