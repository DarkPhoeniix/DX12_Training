
#include "../UnifiedRootSignature.hlsli"

#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"
#include "../DepthFuncs.hlsli"
#include "../PBR.hlsli"

struct PassConstants
{
    uint AlbedoMetallicTextureIndex;
    uint NormalRoughnessTextureIndex;
    uint EmissionTextureIndex;
    uint DepthTextureIndex;
    uint TargetTextureIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

void SetLightParams(in LightDesc light, inout Surface surface);
float CalculateShadowAttenuation_PCF3x3(in LightDesc light, in Surface surface);

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
    Texture2D<float3> EmissionTexture           = ResourceDescriptorHeap[PassCB.EmissionTextureIndex];
    Texture2D<float> DepthTexture               = ResourceDescriptorHeap[PassCB.DepthTextureIndex];
    RWTexture2D<float4> TargetTexture           = ResourceDescriptorHeap[PassCB.TargetTextureIndex];
    
    StructuredBuffer<LightDesc> LightsBuffer    = ResourceDescriptorHeap[FrameCB.LightsBufferIndex];
    
    // Setup surface
    Surface surface;
    uint3 textureLocation = uint3(DTid.xy, 0);
    float depth = DepthTexture.Load(textureLocation);
    surface.Position = ReconstructPosW(depth, DTid.xy, FrameCB.WindowSize, FrameCB.InvProjection, FrameCB.InvView);
    surface.NDCPosition = mul(surface.Position, FrameCB.ViewProjection);
    surface.Albedo = AlbedoMetallicTexture.Load(textureLocation).rgb;
    surface.Metallic = AlbedoMetallicTexture.Load(textureLocation).a;
    surface.Normal = NormalRoughnessTexture.Load(textureLocation).xyz;
    surface.Roughness = NormalRoughnessTexture.Load(textureLocation).a;
    surface.Emission = EmissionTexture.Load(textureLocation);
    surface.FinalColor = TargetTexture.Load(textureLocation);
    
    // Direct lighting
    {
        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        // Metallic factor
        F0 = lerp(F0, surface.Albedo, surface.Metallic);
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
            
            float3 lightAttenuation = CalculateAttenuation(light, surface) * light.Color * light.Intesity;
            float3 diffuseColor = surface.Albedo * kD;
            float3 surfaceColor = (diffuseColor + specularColor) * surface.NdotL;
            
            float3 lightingModel = surfaceColor * lightAttenuation + surface.Emission;
            
            float shadowAttenuation = CalculateShadowAttenuation_PCF3x3(light, surface);
            lightingModel *= (LightsBuffer[i].CastShadows != 0) ? shadowAttenuation : 1.0f;
            
            surface.FinalColor += float4(lightingModel, 0.0f);
        }
    }
    
    TargetTexture[DTid.xy] = surface.FinalColor;
}

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
    surface.Reflect = float4(reflect(-eyeDir, surface.Normal), 0.0f);
    surface.DistanceToL = min(light.Range, distanceToLight);
    surface.NdotV = max(dot(surface.Normal, eyeDir), 0.0f);
    surface.NdotL = max(dot(surface.Normal, lightDirection), 0.0f);
    surface.NdotH = max(dot(surface.Normal, halfway), 0.0f);
}

float CalculateShadowAttenuation_PCF3x3(in LightDesc light, in Surface surface)
{
    uint shadowMapTextureIndex = light.ShadowMapIndex;
    if (light.Type == LIGHT_TYPE_POINT)
    {
        TextureCube shadowMap = ResourceDescriptorHeap[shadowMapTextureIndex];
        return CalculatePointLightShadowAttenuation(shadowMap, ShadowClampSampler, light, surface);
    }
    else if (light.Type == LIGHT_TYPE_SPOT)
    {
        Texture2D shadowMap = ResourceDescriptorHeap[shadowMapTextureIndex];
        return CalculateSpotLightShadowAttenuation(shadowMap, ShadowClampSampler, light, surface);
    }
    
    return 1.0f;
}
