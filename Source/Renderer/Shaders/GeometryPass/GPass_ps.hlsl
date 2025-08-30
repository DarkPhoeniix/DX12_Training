
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"

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
    float4 AlbedoMetalness  : SV_Target0;
    float4 NormalRougness   : SV_Target1;
};

struct PassConstants
{
    uint Instance;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

[earlydepthstencil]
PSOutput main(PSinput IN)
{
    StructuredBuffer<ModelDesc> Instances = ResourceDescriptorHeap[FrameCB.InstancesBufferIndex];
    ModelDesc Model = Instances[PassCB.Instance];
    
    Texture2D AlbedoTexture     = ResourceDescriptorHeap[Model.AlbedoTextureIndex];
    Texture2D NormalTexture     = ResourceDescriptorHeap[Model.NormalTextureIndex];
    Texture2D MetalnessTexture  = ResourceDescriptorHeap[Model.MetalnessTextureIndex];
    Texture2D RoughnessTexture  = ResourceDescriptorHeap[Model.RoughnessTextureIndex];
    
    // Sample textures
    float2 uv                   = IN.Texture;
    uv.y                        = 1.0f - uv.y;
    
    float3 albedo               = AlbedoTexture.Sample(LinearWrapSampler, uv).rgb;
    float3 normalMap            = NormalTexture.Sample(PointWrapSampler, uv).rgb;
    float metalness             = MetalnessTexture.Sample(PointWrapSampler, uv).x;
    float roughness             = RoughnessTexture.Sample(PointWrapSampler, uv).x;
    roughness                   = max(0.05f, roughness); // Set minimum to 0.05 to avoid some visual artifacts in PBR
    
    // Calculate the TBN matrix and a new normal vector
    float3 normal               = normalize(IN.Normal.xyz);
    float3 tangent              = normalize(IN.Tangent.xyz);
    float3 bitangent            = normalize(IN.Bitangent.xyz);
    float3x3 TBN                = float3x3(tangent, bitangent, normal);
    
    float3 finalNormal          = normalize(mul(2.0f * normalMap - 1.0f, TBN));

    // Setup output buffer
    PSOutput output;
    output.AlbedoMetalness      = float4(albedo, metalness);
    output.NormalRougness       = float4(finalNormal, roughness);
    
    return output;
}
