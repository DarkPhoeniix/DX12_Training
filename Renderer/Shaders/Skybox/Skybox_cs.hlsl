
#include "Skybox_rootsig.hlsli"
#include "../Common.hlsli"

#include "../DepthFuncs.hlsli"

Texture2D<float4> DepthTexture      : register(t1);
Texture2D<float4> SkyboxTexture : register(t2);
RWTexture2D<float4> TargetTexture   : register(u0);

const static float Pi = 3.1415926535897f;
const static float Pi_Inv = 1.0f / Pi;
const static float Pi2_Inv = Pi_Inv * 0.5f;

static const float2 invVals = float2(Pi2_Inv, Pi_Inv);
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.x, v.z), asin(-v.y));
    uv *= invVals;
    uv += 0.5f;
    return uv;
}

SamplerState LinearSampler : register(s0);

[RootSignature(Skybox_RootSig)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x > Scene.WindowSize.x || DTid.y > Scene.WindowSize.y)
    {
        return;
    }
    
    float depth = DepthTexture.Load(uint3(DTid.xy, 0)).r;
    
    if (depth != 1.0f)
    {
        return;
    }
    
    float4 pos = ReconstructPosW(depth, DTid.xy, Scene.WindowSize, Scene.InvProjection, Scene.InvView);
    float4 dir = normalize(pos - Scene.EyePosition);
    uint x, y, z;
    SkyboxTexture.GetDimensions(0, x, y, z);
    float2 skyboxTexel = SampleSphericalMap(dir.xyz);
    //skyboxTexel *= float2(x - 1, y - 1);
    float4 color = SkyboxTexture.SampleLevel(LinearSampler, skyboxTexel, 0);
    
    TargetTexture[DTid.xy] = color;
}
