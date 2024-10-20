
#include "DeferredShading_rootsig.hlsli"

#include "Common.hlsli"
#include "LambertLighting.hlsli"
#include "DepthFuncs.hlsli"

StructuredBuffer<LightDesc> Lights          : register(t0);

Texture2D<float4>   PositionTexture         : register(t1);
Texture2D<float4>   AlbedoMetalnessTexture  : register(t2);
Texture2D<float4>   NormalSpecularTexture   : register(t3);
RWTexture2D<float4> TargetTexture           : register(u0);

[RootSignature(DeferredShading_RootSig)]
[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float depth = PositionTexture.Load(uint3(DTid.xy, 0)).r;
    
    // Setup surface
    Surface surface;
    surface.Positon     = ReconstructPosW(depth, DTid.xy, Scene.WindowSize, Scene.InvProjection, Scene.InvView);
    surface.Albedo      = float4(AlbedoMetalnessTexture.Load(uint3(DTid.xy, 0)).rgb, 1.0f);
    surface.Metalness   = AlbedoMetalnessTexture.Load(uint3(DTid.xy, 0)).a;
    surface.Normal      = float4(NormalSpecularTexture.Load(uint3(DTid.xy, 0)).xyz, 0.0f);

    surface.FinalColor  = CalculateAmbient(surface);
    for (int i = 0; i < Scene.LightsNum; ++i)
    {
        surface.FinalColor += CalculateDiffuse(surface, Lights[i]);
    }
    
    TargetTexture[DTid.xy]  = surface.FinalColor;
}
