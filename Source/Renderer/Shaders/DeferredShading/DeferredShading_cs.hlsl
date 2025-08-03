
#include "../UnifiedRootSignature.hlsli"

#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"
#include "../DepthFuncs.hlsli"
#include "../PBR.hlsli"

struct PassConstants
{
    uint AlbedoMetallicTextureIndex;
    uint NormalRoughnessTextureIndex;
    uint DepthTextureIndex;
    uint TargetTextureIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

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
    float3 eyeDir = normalize(FrameCB.EyePosition - surface.Position).xyz;
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
        TextureCube shadowMap = ResourceDescriptorHeap[shadowMapTextureIndex];
        return CalculatePointLightShadowAttenuation(shadowMap, ShadowClampSampler, light, surface);
    }
    else if (light.Type == 2)
    {
        Texture2D shadowMap = ResourceDescriptorHeap[shadowMapTextureIndex];
        return CalculateSpotLightShadowAttenuation(shadowMap, ShadowClampSampler, light, surface);
    }
    
    return 1.0f;
}

[RootSignature(URootSignature)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x > FrameCB.WindowSize.x || DTid.y > FrameCB.WindowSize.y)
    {
        return;
    }
    
    Texture2D AlbedoMetallicTexture             = ResourceDescriptorHeap[PassCB.AlbedoMetallicTextureIndex];
    Texture2D NormalRoughnessTexture            = ResourceDescriptorHeap[PassCB.NormalRoughnessTextureIndex];
    Texture2D DepthTexture                      = ResourceDescriptorHeap[PassCB.DepthTextureIndex];
    RWTexture2D<float4> TargetTexture           = ResourceDescriptorHeap[PassCB.TargetTextureIndex];
    
    StructuredBuffer<LightDesc> LightsBuffer    = ResourceDescriptorHeap[FrameCB.LightsBufferIndex];
    
    // Setup surface
    Surface surface;
    float depth = DepthTexture.Load(uint3(DTid.xy, 0)).r;
    surface.Position = ReconstructPosW(depth, DTid.xy, FrameCB.WindowSize, FrameCB.InvProjection, FrameCB.InvView);
    surface.NDCPosition = mul(surface.Position, FrameCB.ViewProjection);
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
        
        for (int i = 0; i < FrameCB.LightsNum; ++i)
        {
            LightDesc light = LightsBuffer[i];
            
            // Setup surface data for current light source
            SetLightParams(light, surface);
            
            // PBR - BRDF
            // (F * G * D) / (4 * NdotL * NdotV)
            float3 F = FresnelSchlick(surface, F0);
            float G = GeometrySmith(surface);
            float D = CalculateSpecular(surface);
            
            float3 specularColor = (F * G * D) / max(0.00001f, (4.0f * surface.NdotL * surface.NdotV));
            
            float3 lightAttenuation = CalculateAttenuation(light, surface) * light.Color.rgb * light.Intesity;
            float3 diffuseColor = surface.Albedo.rgb * kD;
            float3 surfaceColor = (diffuseColor + specularColor) * surface.NdotL;
            
            float3 lightingModel = surfaceColor * lightAttenuation;
            
            //float shadowAttenuation = CalculateShadowAttenuation_PCF3x3(Lights[i], surface);
            float shadowAttenuation = 1.0f;
            lightingModel *= (LightsBuffer[i].CastShadows != 0) ? shadowAttenuation : 1.0f;
            
            surface.FinalColor += float4(lightingModel, 0.0f);
        }
    }
    
    TargetTexture[DTid.xy] = surface.FinalColor;
}