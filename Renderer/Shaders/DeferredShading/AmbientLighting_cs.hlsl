
#define AmbientLighting_RootSig \
	"RootFlags " \
	"(0), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_ALL), " \
    "CBV(b1, visibility = SHADER_VISIBILITY_ALL), " \
    "SRV(t0, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t1), visibility=SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(SRV(t2), visibility=SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(SRV(t3), visibility=SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(SRV(t4), visibility=SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(SRV(t5), visibility=SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(SRV(t6), visibility=SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(UAV(u0), visibility=SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_POINT_MIP_LINEAR)"

#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"
#include "../DepthFuncs.hlsli"
#include "../PBR.hlsli"

StructuredBuffer<LightDesc> Lights : register(t0);

Texture2D<float4> PositionTexture : register(t1);
Texture2D<float4> AlbedoMetallicTexture : register(t2);
Texture2D<float4> NormalRoughnessTexture : register(t3);
TextureCube DiffuseIrradiance : register(t4);
TextureCube PreFilteredMap : register(t5);
Texture2D<float2> brdfLUT : register(t6);
RWTexture2D<float4> TargetTexture : register(u0);

SamplerState PointSampler : register(s0);

[RootSignature(AmbientLighting_RootSig)]
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
    surface.FinalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float3 eyeDir = normalize(Scene.EyePosition - surface.Position).xyz;
    surface.NdotV = max(dot(surface.Normal.xyz, eyeDir), 0.0f);
    surface.Reflect = float4(reflect(-eyeDir, surface.Normal.xyz), 0.0f);
    
    // Ambient lighting
    {
#ifdef USE_IBL
        float w, h, mipLevels;
        PreFilteredMap.GetDimensions(0, w, h, mipLevels);
        
        float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), surface.Albedo.rgb, surface.Metallic);
        float3 F = FresnelSchlickRoughness(surface.NdotV, F0, surface.Roughness);
        
        float3 kS = F;
        float3 kD = (1.0f - kS) * (1.0f - surface.Metallic);
        
        // diffuse
        float3 irradiance = DiffuseIrradiance.SampleLevel(PointSampler, surface.Normal.xyz, 0.0f).rgb;
        float3 diffuseIBL = kD * irradiance * surface.Albedo.rgb;
        
        // specular
        float3 prefilteredColor = PreFilteredMap.SampleLevel(PointSampler, surface.Reflect.xyz, surface.Roughness * (mipLevels - 1.0f)).rgb;
        float2 envBRDF = brdfLUT.SampleLevel(PointSampler, float2(surface.NdotV, surface.Roughness), 0.0f).rg;
        float3 specularIBL = prefilteredColor * (F * envBRDF.x + envBRDF.y);    
        
        // combine
        float3 ambient = (diffuseIBL + specularIBL);
#else
        float3 ambient = surface.Albedo.rgb * 0.1f;
#endif
        surface.FinalColor += float4(ambient, 0.0f);
    }
    
    TargetTexture[DTid.xy] = surface.FinalColor;
}
