
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"
#include "../DepthFuncs.hlsli"
#include "../PBR.hlsli"

struct PassConstants
{
    uint DepthTextureIndex;
    uint AlbedoMetallicTextureIndex;
    uint NormalRoughnessTextureIndex;
    uint DiffuseIrradianceCubemapIndex;
    uint PreFilteredEnvironmentCubemapIndex;
    uint BRDFLUTTextureIndex;
    uint TargetTextureIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

[RootSignature(URootSignature)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x > FrameCB.WindowSize.x || DTid.y > FrameCB.WindowSize.y)
    {
        return;
    }
    
    StructuredBuffer<LightDesc> Lights  = ResourceDescriptorHeap[FrameCB.LightsBufferIndex];
    
    Texture2D DepthTexture              = ResourceDescriptorHeap[PassCB.DepthTextureIndex];
    Texture2D AlbedoMetallicTexture     = ResourceDescriptorHeap[PassCB.AlbedoMetallicTextureIndex];
    Texture2D NormalRoughnessTexture    = ResourceDescriptorHeap[PassCB.NormalRoughnessTextureIndex];
    TextureCube DiffuseIrradiance       = ResourceDescriptorHeap[PassCB.DiffuseIrradianceCubemapIndex];
    TextureCube PreFilteredMap          = ResourceDescriptorHeap[PassCB.PreFilteredEnvironmentCubemapIndex];
    Texture2D<float2> brdfLUT           = ResourceDescriptorHeap[PassCB.BRDFLUTTextureIndex];
    RWTexture2D<float4> TargetTexture   = ResourceDescriptorHeap[PassCB.TargetTextureIndex];
    
    // Setup surface
    Surface surface;
    float depth = DepthTexture.Load(uint3(DTid.xy, 0)).r;
    surface.Position = ReconstructPosW(depth, DTid.xy, FrameCB.WindowSize, FrameCB.InvProjection, FrameCB.InvView);
    surface.NDCPosition = mul(surface.Position, FrameCB.ViewProjection);
    surface.Albedo = float4(AlbedoMetallicTexture.Load(uint3(DTid.xy, 0)).rgb, 1.0f);
    surface.Normal = float4(NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).xyz, 0.0f);
    surface.Metallic = AlbedoMetallicTexture.Load(uint3(DTid.xy, 0)).a;
    surface.Roughness = NormalRoughnessTexture.Load(uint3(DTid.xy, 0)).a;
    surface.FinalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float3 eyeDir = normalize(FrameCB.EyePosition - surface.Position).xyz;
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
        float3 irradiance = DiffuseIrradiance.SampleLevel(PointClampSampler, surface.Normal.xyz, 0.0f).rgb;
        float3 diffuseIBL = kD * irradiance * surface.Albedo.rgb;
        
        // specular
        float3 prefilteredColor = PreFilteredMap.SampleLevel(PointClampSampler, surface.Reflect.xyz, surface.Roughness * (mipLevels - 1.0f)).rgb;
        float2 envBRDF = brdfLUT.SampleLevel(PointClampSampler, float2(surface.NdotV, surface.Roughness), 0.0f).rg;
        float3 specularIBL = prefilteredColor * (F * envBRDF.x + envBRDF.y);    
        
        // combine
        float3 ambient = (diffuseIBL + specularIBL);
#else
        float3 ambient = surface.Albedo.rgb * 0.01f;
#endif
        surface.FinalColor += float4(ambient, 0.0f);
    }
    
    TargetTexture[DTid.xy] = surface.FinalColor;
}
