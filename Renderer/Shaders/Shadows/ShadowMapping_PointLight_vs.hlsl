
#define ShadowMapping_Point_RootSig \
	"RootFlags " \
	"( " \
		"ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS " \
	"), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_ALL), " \
    "CBV(b1, visibility = SHADER_VISIBILITY_VERTEX), " \
    "SRV(t0, visibility = SHADER_VISIBILITY_VERTEX), " \
	"SRV(t1, visibility = SHADER_VISIBILITY_GEOMETRY), " \
    "RootConstants(num32BitConstants=1, b3, visibility=SHADER_VISIBILITY_ALL)"

#include "../Common.hlsli"
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
    float4 Position : SV_Position;
};

struct BoneDesc
{
    row_major matrix Transform;
};

StructuredBuffer<BoneDesc> Bones : register(t0);

[RootSignature(ShadowMapping_Point_RootSig)]
VSOutput main(VSinput IN)
{
    row_major matrix boneTransform = float4x4(
        float4(1.0f, 0.0f, 0.0f, 0.0f),
        float4(0.0f, 1.0f, 0.0f, 0.0f),
        float4(0.0f, 0.0f, 1.0f, 0.0f),
        float4(0.0f, 0.0f, 0.0f, 1.0f));
    //if (Model.useSkinning)
    //{
    //    boneTransform  = Bones[Vertices[IN.VertexID].BoneIds[0]].Transform * Vertices[IN.VertexID].BoneWeights[0];
    //    boneTransform += Bones[Vertices[IN.VertexID].BoneIds[1]].Transform * Vertices[IN.VertexID].BoneWeights[1];
    //    boneTransform += Bones[Vertices[IN.VertexID].BoneIds[2]].Transform * Vertices[IN.VertexID].BoneWeights[2];
    //    boneTransform += Bones[Vertices[IN.VertexID].BoneIds[3]].Transform * Vertices[IN.VertexID].BoneWeights[3];
    //}
    
    float4 positionWS = float4(IN.Position, 1.0f);
    positionWS = mul(positionWS, boneTransform);
    positionWS = mul(positionWS, Model.Transform);
    
    VSOutput output;
    output.Position = positionWS;
    
    return output;
}
