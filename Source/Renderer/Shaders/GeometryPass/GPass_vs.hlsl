
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"

struct VSinput
{
    float4 Position         : POSITION0;
    float4 Normal           : NORMAL0;
    float4 Tangent          : TANGENT0;
    float2 Texture          : TEXCOORD0;
    uint4 BoneIds           : BONE_IDS;
    float4 BoneWeights      : BONE_WEIGHTS;
};

struct VSOutput
{
    float4 WorldPosition    : POSITION0;
    float4 Position         : SV_Position;
    float4 Normal           : NORMAL0;
    float4 Tangent          : TANGENT0;
    float4 Bitangent        : BITANGENT0;
    float2 Texture          : TEXCOORD0;
};

struct BoneDesc
{
    row_major matrix Transform;
};

struct PassConstants
{
    uint Instance;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

[RootSignature(URootSignature)]
VSOutput main(VSinput IN)
{
    StructuredBuffer<ModelDesc> Instances = ResourceDescriptorHeap[FrameCB.InstancesBufferIndex];
    ModelDesc Model = Instances[PassCB.Instance];
    
    row_major matrix boneTransform = float4x4(
        float4(1.0f, 0.0f, 0.0f, 0.0f),
        float4(0.0f, 1.0f, 0.0f, 0.0f),
        float4(0.0f, 0.0f, 1.0f, 0.0f),
        float4(0.0f, 0.0f, 0.0f, 1.0f));
    if (Model.BonesBufferIndex != -1)
    {
        StructuredBuffer<BoneDesc> Bones = ResourceDescriptorHeap[Model.BonesBufferIndex];
        boneTransform       = Bones[IN.BoneIds[0]].Transform * IN.BoneWeights[0];
        boneTransform      += Bones[IN.BoneIds[1]].Transform * IN.BoneWeights[1];
        boneTransform      += Bones[IN.BoneIds[2]].Transform * IN.BoneWeights[2];
        boneTransform      += Bones[IN.BoneIds[3]].Transform * IN.BoneWeights[3];
    }
    
    float4 objectPosition   = mul(IN.Position, boneTransform);
    float4 normal           = normalize(mul(IN.Normal, boneTransform));
    float4 tangent          = normalize(mul(IN.Tangent, boneTransform));
    
    float4 worldPosition    = mul(objectPosition, Model.Transform);
    
    VSOutput output;
    output.WorldPosition    = worldPosition;
    output.Position         = mul(worldPosition, FrameCB.ViewProjection);
    output.Normal           = normalize(mul(normal, Model.Transform));
    output.Tangent          = normalize(mul(tangent, Model.Transform));
    output.Bitangent        = float4(normalize(IN.Tangent.w * cross(normal.xyz, tangent.xyz)), 0.0f);
    output.Texture          = IN.Texture;

    return output;
}
