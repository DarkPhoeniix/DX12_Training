
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
        
    surface.ToLight = toLight;
    surface.DistanceToL = min(light.Range, distanceToLight);
    surface.NdotV = max(dot(surface.Normal.xyz, eyeDir), 0.0f);
    surface.NdotL = max(dot(surface.Normal.xyz, lightDirection), 0.0f);
    surface.NdotH = max(dot(surface.Normal.xyz, halfway), 0.0f);
}

float CalculateShadowAttenuation_PCF3x3(in LightDesc light, in Surface surface)
{
    uint shadowMapIndex = 0;
    if (light.Type == LIGHT_TYPE_POINT)
    {
        shadowMapIndex = GetCubeFaceIndex(surface.Position.xyz - light.Position.xyz);
    }
    
    row_major matrix VP = light.ViewProj[shadowMapIndex];
    float4 surfacePos = mul(surface.Position, VP);
    surfacePos /= surfacePos.w;
    
    if (surfacePos.x < -1.0f || surfacePos.x > 1.0f ||
        surfacePos.y < -1.0f || surfacePos.y > 1.0f ||
        surfacePos.z <  0.0f || surfacePos.z > 1.0f)
    {
        return 0.0f;
    }
    
    float3 UVD;
    UVD.x = (surfacePos.x *  0.5f) + 0.5f;
    UVD.y = (surfacePos.y * -0.5f) + 0.5f;
    UVD.z = surfacePos.z - 0.001f;
    
    float2 offsets[9] =
    {
        float2(-1, -1), float2(-1, 0), float2(-1, 1),
        float2( 0, -1), float2( 0, 0), float2( 0, 1),
        float2( 1, -1), float2( 1, 0), float2( 1, 1)
    };
    
    float shadowFactor = 0.0f;
    
    uint shadowMapTextureIndex = light.ShadowMapIndexes[shadowMapIndex];
    [unroll(9)]
    for (uint i = 0; i < 9; ++i)
    {
        shadowFactor += Textures[shadowMapTextureIndex].SampleCmpLevelZero(ShadowSampler, UVD.xy, UVD.z, offsets[i]);
    }
    shadowFactor /= 9.0f;
    
    return shadowFactor;
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
    float depth             = PositionTexture.Load(uint3(DTid.xy, 0)).r;
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
        float  shadowAttenuation = CalculateShadowAttenuation_PCF3x3(Lights[i], surface);
        float3 surfaceColor = (diffuseColor + specularColor) * surface.NdotL;
        
        float3 lightingModel = surfaceColor * lightAttenuation;
        if (Lights[i].CastShadows != 0)
        {
            lightingModel *= shadowAttenuation;
        }
        
        surface.FinalColor += float4(lightingModel, 1.0f);
    }
    
    TargetTexture[DTid.xy] = float4(surface.FinalColor.rgb, 1.0f);
}
