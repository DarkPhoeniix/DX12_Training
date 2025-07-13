
#include "DeferredShading_rootsig.hlsli"

#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"
#include "../DepthFuncs.hlsli"
#include "../PBR.hlsli"

StructuredBuffer<LightDesc> Lights : register(t0);

Texture2D<float4> PositionTexture : register(t1);
Texture2D<float4> AlbedoMetallicTexture : register(t2);
Texture2D<float4> NormalRoughnessTexture : register(t3);
Texture2D Textures2D[] : register(t4, space0);
TextureCube TexturesCube[] : register(t4, space1);
RWTexture2D<float4> TargetTexture : register(u0);

SamplerComparisonState ShadowSampler : register(s0);

void SetLightParams(in LightDesc light, inout Surface surface)
{
    float3 lightDirection;
    float distanceToLight = 0.0f;
    float4 toLight;
    if (light.Type == LIGHT_TYPE_DIRECTIONAL)
    {
        lightDirection = -normalize(light.Direction).xyz;
    }
    else if (light.Type == LIGHT_TYPE_POINT)
    {
        toLight = light.Position - surface.Position;
        lightDirection = normalize(toLight);
        distanceToLight = sqrt(dot(toLight, toLight));
    }
    else if (light.Type == LIGHT_TYPE_SPOT)
    {
        toLight = light.Position - surface.Position;
        lightDirection = normalize(toLight).xyz;
        distanceToLight = sqrt(dot(normalize(toLight), normalize(toLight)));
    }
    float3 eyeDir = normalize(Scene.EyePosition - surface.Position).xyz;
    float3 halfway = normalize(eyeDir + lightDirection);
        
    surface.ViewDirection = float4(eyeDir, 0.0f);
    surface.ToLight = toLight;
    surface.Reflect = float4(reflect(-eyeDir, surface.Normal.xyz), 0.0f);
    surface.DistanceToL = min(light.Range, distanceToLight);
    surface.NdotV = max(dot(surface.Normal.xyz, eyeDir), 0.0f);
    surface.NdotL = max(dot(surface.Normal.xyz, lightDirection), 0.0f);
    surface.NdotH = max(dot(surface.Normal.xyz, halfway), 0.0f);
}

float CalculateShadowAttenuation_PCF3x3(in LightDesc light, in Surface surface)
{
    uint shadowMapTextureIndex = light.ShadowMapIndex;
    if (light.Type == 1)
    {
        return CalculatePointLightShadowAttenuation(TexturesCube[shadowMapTextureIndex], ShadowSampler, light, surface);
    }
    else if (light.Type == 2)
    {
        return CalculateSpotLightShadowAttenuation(Textures2D[shadowMapTextureIndex], ShadowSampler, light, surface);
    }
    
    return 1.0f;
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
    surface.Position = ReconstructPosW(depth, DTid.xy, Scene.WindowSize, Scene.InvProjection, Scene.InvView);
    surface.NDCPosition = mul(surface.Position, Scene.ViewProjection);
    surface.Albedo = float4(AlbedoMetallicTexture.Load(uint3(DTid.xy, 0)).rgb, 1.0f);
    surface.Normal = float4(NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).xyz, 0.0f);
    surface.Metallic = AlbedoMetallicTexture.Load(uint3(DTid.xy, 0)).a;
    surface.Roughness = NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).a;
    surface.FinalColor = TargetTexture.Load(uint3(DTid.xy, 0));
    
    // Direct lighting
    {
        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        // Metallic factor
        F0 = lerp(F0, surface.Albedo.rgb, surface.Metallic);
        float3 kS = max(0.0f, FresnelSchlick(surface, F0));
        float3 kD = 1.0 - kS;
        kD *= 1.0 - surface.Metallic;
        
        for (int i = 0; i < Scene.LightsNum; ++i)
        {
            // Setup surface data for current light source
            SetLightParams(Lights[i], surface);
            
            // PBR - BRDF
            // (F * G * D) / (4 * NdotL * NdotV)
            float3 F = FresnelSchlick(surface, F0);
            float G = GeometrySmith(surface);
            float D = CalculateSpecular(surface);
            
            float3 specularColor = (F * G * D) / max(0.00001f, (4.0f * surface.NdotL * surface.NdotV));
            
            float3 lightAttenuation = CalculateAttenuation(Lights[i], surface) * Lights[i].Color.rgb * Lights[i].Intesity;
            float3 diffuseColor = surface.Albedo.rgb * kD;
            float3 surfaceColor = (diffuseColor + specularColor) * surface.NdotL;
            
            float3 lightingModel = surfaceColor * lightAttenuation;
            
            float shadowAttenuation = CalculateShadowAttenuation_PCF3x3(Lights[i], surface);
            lightingModel *= (Lights[i].CastShadows != 0) ? shadowAttenuation : 1.0f;
            
            surface.FinalColor += float4(lightingModel, 0.0f);
        }
    }
    
    TargetTexture[DTid.xy] = surface.FinalColor;
}