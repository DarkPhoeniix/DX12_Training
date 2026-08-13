
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"

struct VSinput
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float2 Texture      : TEXCOORD;
    uint4  BoneIds      : BONE_IDS;
    float4 BoneWeights  : BONE_WEIGHTS;
};

struct VSOutput
{
    float4 Position     : SV_Position;
};

struct BoneDesc
{
    row_major matrix Transform;
};

struct PassConstants
{
    uint InstanceIndex;
    uint LightIndex;
};

URootConstants(PassConstants, PassCB);

[RootSignature(URootSignature)]
VSOutput main(VSinput IN)
{
    StructuredBuffer<ModelDesc> Instances = ResourceDescriptorHeap[FrameCB.InstancesBufferIndex];
    StructuredBuffer<LightDesc> LightsBuffer = ResourceDescriptorHeap[FrameCB.LightsBufferIndex];
    ModelDesc Model = Instances[PassCB.InstanceIndex];
    
    row_major matrix boneTransform = float4x4(
        float4(1.0f, 0.0f, 0.0f, 0.0f),
        float4(0.0f, 1.0f, 0.0f, 0.0f),
        float4(0.0f, 0.0f, 1.0f, 0.0f),
        float4(0.0f, 0.0f, 0.0f, 1.0f));
    if (Model.BonesBufferIndex != -1)
    {
        StructuredBuffer<BoneDesc> Bones = ResourceDescriptorHeap[Model.BonesBufferIndex];
        boneTransform = Bones[IN.BoneIds[0]].Transform * IN.BoneWeights[0];
        boneTransform += Bones[IN.BoneIds[1]].Transform * IN.BoneWeights[1];
        boneTransform += Bones[IN.BoneIds[2]].Transform * IN.BoneWeights[2];
        boneTransform += Bones[IN.BoneIds[3]].Transform * IN.BoneWeights[3];
    }
    
    float4 positionWS   = float4(IN.Position, 1.0f);
    positionWS          = mul(positionWS, boneTransform);
    positionWS          = mul(positionWS, Model.Transform);
    float4 positionLS   = mul(positionWS, LightsBuffer[PassCB.LightIndex].ViewProj[0]);
    
    VSOutput output;
    output.Position = positionLS;
    
    return output;
}
