
#include "DeferredShading_rootsig.hlsli"

#include "Common.hlsli"
#include "LightingCommon.hlsli"
#include "DepthFuncs.hlsli"
#include "PBR.hlsli"

StructuredBuffer<LightDesc> Lights          : register(t0);

Texture2D<float4>   PositionTexture         : register(t1);
Texture2D<float4>   AlbedoMetalnessTexture  : register(t2);
Texture2D<float4>   NormalRoughnessTexture  : register(t3);
Texture2D<float4>   ShadowMap               : register(t4);
RWTexture2D<float4> TargetTexture           : register(u0);

SamplerState LinearSampler : register(s0);

float CalculateShadowAttenuation(LightDesc light, Surface surface)
{
    float4 surfacePos = mul(surface.Position, mul(light.View, light.Proj));
    surfacePos /= surfacePos.w;
    
    if (surfacePos.x < -1.0f || surfacePos.x > 1.0f ||
        surfacePos.y < -1.0f || surfacePos.y > 1.0f ||
        surfacePos.z <  0.0f || surfacePos.z > 1.0f)
        return 0.0f;
    
    float2 UV;
    UV.x = (surfacePos.x /  2.0f) + 0.5f;
    UV.y = (surfacePos.y / -2.0f) + 0.5f;
    surfacePos.z -= 0.001f;
    
    float depth = ShadowMap.SampleLevel(LinearSampler, UV, 0).r;
    
    if (depth < surfacePos.z)
    {
        return 0.0f;
    }

    return 1.0f;
}

void SetLightParams(in LightDesc light, inout Surface surface)
{
    float3 eyeDir = normalize(Scene.EyePosition - surface.Position).xyz;
    
    float3 lightDirection;
    float distanceToLight = 0.0f;
    if (light.Type == LIGHT_TYPE_DIRECTIONAL)
    {
        lightDirection = -normalize(light.Direction).xyz;
    }
    else if (light.Type == LIGHT_TYPE_POINT)
    {
        float3 direction = (light.Position - surface.Position).xyz;
        lightDirection = normalize(direction);
        distanceToLight = sqrt(dot(direction, direction));
    }
    else if (light.Type == LIGHT_TYPE_SPOT)
    {
        float3 direction = (light.Position - surface.Position).xyz;
        lightDirection = normalize(direction).xyz;
        distanceToLight = sqrt(dot(normalize(direction), normalize(direction)));
    }
    float3 halfway = normalize(eyeDir + lightDirection);
        
    surface.DistanceToL = min(light.Range, distanceToLight);
    surface.NdotV = max(dot(surface.Normal.xyz, eyeDir), 0.0f);
    surface.NdotL = max(dot(surface.Normal.xyz, lightDirection), 0.0f);
    surface.NdotH = max(dot(surface.Normal.xyz, halfway), 0.0f);
}

[RootSignature(DeferredShading_RootSig)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x > Scene.WindowSize.x || DTid.y > Scene.WindowSize.y)
    {
        return;
    }
    
    float depth             = PositionTexture.Load(uint3(DTid.xy, 0)).r;
    
    // Setup surface
    Surface surface;
    surface.Position        = ReconstructPosW(depth, DTid.xy, Scene.WindowSize, Scene.InvProjection, Scene.InvView);
    surface.NDCPosition     = mul(surface.Position, Scene.ViewProjection);
    surface.Albedo          = float4(AlbedoMetalnessTexture.Load(uint3(DTid.xy, 0)).rgb, 1.0f);
    surface.Normal          = float4(NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).xyz, 0.0f);
    surface.Metalness       = AlbedoMetalnessTexture.Load(uint3(DTid.xy, 0)).a;
    surface.Roughness       = NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).a;
    
    surface.FinalColor = 0.05f * surface.Albedo; // Ambient
    for (int i = 0; i < Scene.LightsNum; ++i)
    {
        SetLightParams(Lights[i], surface);
        
        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        F0 = lerp(F0, surface.Albedo.rgb, surface.Metalness);
        
        // (F * G * D) / (4 * NdotL * NdotV)
        float3 F = FresnelSchlick(surface, F0);
        float G = GeometrySmith(surface);
        float D = CalculateSpecular(surface);
        float3 cookTorrance = (F * G * D) / max(0.00001f, (4.0f * surface.NdotL * surface.NdotV));
        
        float3 diffuseColor = surface.Albedo.rgb * (1.0f - surface.Metalness);
        float lightAttenuation = CalculateAttenuation(Lights[i], surface);
        float3 lightingModel = (diffuseColor + cookTorrance) * surface.NdotL * lightAttenuation * Lights[i].Color.rgb * CalculateShadowAttenuation(Lights[i], surface);
        
        float4 finalDiffuse = float4(lightingModel, 1.0f);
        
        surface.FinalColor += finalDiffuse;
    }
    
    TargetTexture[DTid.xy] = float4(surface.FinalColor.rgb, 1.0f);
}
