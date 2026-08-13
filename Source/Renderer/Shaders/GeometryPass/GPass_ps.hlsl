
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"

#define ALPHA_THRESHOLD 0.5f

struct PSinput
{
    float4 WorldPosition    : POSITION0;
    float4 Position         : SV_Position;
    float4 Normal           : NORMAL0;
    float4 Tangent          : TANGENT0;
    float4 Bitangent        : BITANGENT0;
    float2 Texture          : TEXCOORD0;
};

struct PSOutput
{
    float4 AlbedoMetallic   : SV_Target0;
    float4 NormalRougness   : SV_Target1;
    float3 Emission         : SV_Target2;
};

struct PassConstants
{
    uint Instance;
};

URootConstants(PassConstants, PassCB);

PSOutput main(PSinput IN)
{
    StructuredBuffer<ModelDesc> Instances = ResourceDescriptorHeap[FrameCB.InstancesBufferIndex];
    ModelDesc Model = Instances[PassCB.Instance];
    
    // Sample textures
    float2 uv = IN.Texture;
    uv.y = 1.0f - uv.y;
    
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 emission = float3(0.0f, 0.0f, 0.0f);
    float3 normalMap = float3(0.0f, 0.0f, 0.0f);
    float metallic = 0.0f;
    float roughness = 0.0f;
    
    if (Model.AlbedoTextureIndex != -1)
    {
        Texture2D AlbedoTexture = ResourceDescriptorHeap[Model.AlbedoTextureIndex];
        color = AlbedoTexture.Sample(AnisotropicWrapSampler, uv).rgba;
    }
    else
    {
        color = Model.AlbedoColor;
    }
    clip(color.a - ALPHA_THRESHOLD); // Discard pixel if alpha is below threshold
    
    if (Model.EmissionTextureIndex != -1)
    {
        Texture2D EmissionTexture = ResourceDescriptorHeap[Model.EmissionTextureIndex];
        emission = EmissionTexture.Sample(AnisotropicWrapSampler, uv).rgb * EmissionTexture.Sample(LinearWrapSampler, uv).a * Model.EmissiveIntensity;
    }
    else
    {
        emission = Model.EmissiveColor * Model.EmissiveIntensity;
    }
    
    if (Model.NormalTextureIndex != -1)
    {
        Texture2D NormalTexture = ResourceDescriptorHeap[Model.NormalTextureIndex];
        normalMap = NormalTexture.Sample(LinearWrapSampler, uv).rgb;
    }
    else
    {
        normalMap = float3(0.5f, 0.5f, 1.0f); // Default normal map value
    }
    
    if (Model.MetallicTextureIndex != -1)
    {
        Texture2D MetallicTexture = ResourceDescriptorHeap[Model.MetallicTextureIndex];
        metallic = MetallicTexture.Sample(LinearWrapSampler, uv).x;
    }
    else
    {
        metallic = Model.MetallicValue;
    }
    
    if (Model.RoughnessTextureIndex != -1)
    {
        Texture2D RoughnessTexture = ResourceDescriptorHeap[Model.RoughnessTextureIndex];
        roughness = RoughnessTexture.Sample(LinearWrapSampler, uv).x;
    }
    else
    {
        roughness = Model.RoughnessValue;
    }
    
    // Calculate the TBN matrix and a new normal vector
    float3 normal               = normalize(IN.Normal.xyz);
    float3 tangent              = normalize(IN.Tangent.xyz);
    float3 bitangent            = normalize(IN.Bitangent.xyz);
    float3x3 TBN                = float3x3(tangent, bitangent, normal);
    
    float3 finalNormal          = normalize(mul(2.0f * normalMap - 1.0f, TBN));

    // Setup output buffer
    PSOutput output;
    output.AlbedoMetallic       = float4(color.rgb, metallic);
    output.NormalRougness       = float4(finalNormal, roughness);
    output.Emission             = emission;
    
    return output;
}
