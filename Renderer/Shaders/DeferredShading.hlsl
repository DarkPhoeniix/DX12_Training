
#include "DeferredShading_rootsig.hlsli"

#include "Common.hlsli"
#include "LightingCommon.hlsli"
#include "DepthFuncs.hlsli"
#include "PBR.hlsli"

StructuredBuffer<LightDesc> Lights                  : register(t0);

Texture2D<float4>           PositionTexture         : register(t1);
Texture2D<float4>           AlbedoMetalnessTexture  : register(t2);
Texture2D<float4>           NormalRoughnessTexture  : register(t3);
Texture2D                   Textures[]              : register(t4);
RWTexture2D<float4>         TargetTexture           : register(u0);

SamplerComparisonState      ShadowSampler           : register(s0);

void SetLightParams(in LightDesc light, inout Surface surface)
{
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
    float3 eyeDir = normalize(Scene.EyePosition - surface.Position).xyz;
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
    
    // Setup surface
    Surface surface;
    float depth = PositionTexture.Load(uint3(DTid.xy, 0)).r;
    surface.Position        = ReconstructPosW(depth, DTid.xy, Scene.WindowSize, Scene.InvProjection, Scene.InvView);
    surface.NDCPosition     = mul(surface.Position, Scene.ViewProjection);
    surface.Albedo          = float4(AlbedoMetalnessTexture.Load(uint3(DTid.xy, 0)).rgb, 1.0f);
    surface.Normal          = float4(NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).xyz, 0.0f);
    surface.Metalness       = AlbedoMetalnessTexture.Load(uint3(DTid.xy, 0)).a;
    surface.Roughness       = NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).a;
    
    surface.FinalColor = 0.05f * surface.Albedo; // Ambient
    for (int i = 0; i < Scene.LightsNum; ++i)
    {
        // Setup surface data for current light source
        SetLightParams(Lights[i], surface);
        
        // Metalness factor
        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        F0 = lerp(F0, surface.Albedo.rgb, surface.Metalness);
        
        // PBR - BRDF
        // (F * G * D) / (4 * NdotL * NdotV)
        float3 F = FresnelSchlick(surface, F0);
        float G = GeometrySmith(surface);
        float D = CalculateSpecular(surface);
        float3 specularColor = (F * G * D) / max(0.00001f, (4.0f * surface.NdotL * surface.NdotV));
        
        float3 diffuseColor = surface.Albedo.rgb * (1.0f - surface.Metalness);
        float3 lightAttenuation = CalculateAttenuation(Lights[i], surface) * Lights[i].Color.rgb;
        float  shadowAttenuation = CalculateShadowAttenuation_PCF3x3(Textures[Lights[i].ShadowMapIndex], ShadowSampler, Lights[i], surface);
        float3 surfaceColor = (diffuseColor + specularColor) * surface.NdotL;
        
        float3 lightingModel = surfaceColor * lightAttenuation * shadowAttenuation;
        
        surface.FinalColor += float4(lightingModel, 1.0f);
    }
    
    TargetTexture[DTid.xy] = float4(surface.FinalColor.rgb, 1.0f);
}
