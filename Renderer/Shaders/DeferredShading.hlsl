
#include "DeferredShading_rootsig.hlsli"

#include "Common.hlsli"
#include "LightingCommon.hlsli"
#include "DepthFuncs.hlsli"
#include "PBR.hlsli"

StructuredBuffer<LightDesc> Lights          : register(t0);

Texture2D<float4>   PositionTexture         : register(t1);
Texture2D<float4>   AlbedoMetalnessTexture  : register(t2);
Texture2D<float4>   NormalRoughnessTexture  : register(t3);
RWTexture2D<float4> TargetTexture           : register(u0);

[RootSignature(DeferredShading_RootSig)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float depth             = PositionTexture.Load(uint3(DTid.xy, 0)).r;
    
    // Setup surface
    Surface surface;
    surface.Positon         = ReconstructPosW(depth, DTid.xy, Scene.WindowSize, Scene.InvProjection, Scene.InvView);
    surface.Albedo          = float4(AlbedoMetalnessTexture.Load(uint3(DTid.xy, 0)).rgb, 1.0f);
    surface.Normal          = float4(NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).xyz, 0.0f);
    surface.Metalness       = AlbedoMetalnessTexture.Load(uint3(DTid.xy, 0)).a;
    surface.Roughness       = NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).a;
    
    float3 eyeDir = normalize(Scene.EyePosition - surface.Positon);
    
    surface.FinalColor = 0.01f * surface.Albedo;
    for (int i = 0; i < Scene.LightsNum; ++i)
    {
        float3 lightDirection;
        if (Lights[i].Type == LIGHT_TYPE_DIRECTIONAL)
        {
            lightDirection = -normalize(Lights[i].Direction);
        }
        else if (Lights[i].Type == LIGHT_TYPE_POINT)
        {
            lightDirection = normalize(Lights[i].Position - surface.Positon);
        }
        float3 halfway = normalize(eyeDir + lightDirection);
        
        float NdotV = max(dot(surface.Normal.xyz, eyeDir), 0.0f);
        float NdotL = max(dot(surface.Normal.xyz, lightDirection), 0.0f);
        float NdotH = max(dot(surface.Normal.xyz, halfway), 0.0f);
        
        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        F0 = lerp(F0, surface.Albedo.rgb, surface.Metalness);
        
        // (F * G * D) / (4 * NdotL * NdotV)
        float3 cookTorrance = (CalculateSpecular(surface, Lights[i]) * fresnelSchlick(NdotH, F0) * GeometrySmith(surface, Lights[i])) / (4 * NdotL * NdotV);
        
        float3 diffuseColor = surface.Albedo.rgb * (1.0f - surface.Metalness);
        float3 lightingModel = (diffuseColor + cookTorrance);
        lightingModel *= NdotL;
        float4 finalDiffuse = float4(lightingModel, 1.0f);
        
        surface.FinalColor += finalDiffuse;
    }
    
    TargetTexture[DTid.xy]  = surface.Albedo;
}
