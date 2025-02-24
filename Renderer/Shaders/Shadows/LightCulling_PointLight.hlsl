
#define LightCulling_PointLight_RootSig \
	"RootFlags " \
	"( " \
		"ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS " \
	"), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_ALL), " \
    "CBV(b1, visibility = SHADER_VISIBILITY_ALL), " \
	"SRV(t0, visibility = SHADER_VISIBILITY_ALL), " \
    "RootConstants(num32BitConstants = 1, b2, visibility = SHADER_VISIBILITY_ALL), " \
	"SRV(t1, visibility = SHADER_VISIBILITY_ALL), " \
	"SRV(t2, visibility = SHADER_VISIBILITY_ALL), " \
	"DescriptorTable(UAV(u0, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL)"

#include "../Common.hlsli"
#include "../LightingCommon.hlsli"

#define THREAD_BLOCK_SIZE 4

struct IndirectCommand
{
    uint2 Vertex0BufferAddress;
    uint VB0_Size;
    uint VB0_Stride;
    uint2 Vertex1BufferAddress;
    uint VB1_Size;
    uint VB1_Stride;
    uint2 IndexBufferAddress;
    uint IB_Size;
    uint IB_Stride;
    uint2 SceneBufferAddress;
    uint2 ModelBufferAddress;
    uint2 BonesBufferAddress;
    uint2 LightBufferAddress;
    uint  LightIndex;
    
    uint4 DrawArguments;
    
    uint pad;
};

struct MinMax
{
    float4 Min;
    float4 Max;
};

cbuffer Contants                                        : register(b2)
{
    uint CommandsCount;
}
StructuredBuffer<LightDesc> Lights						: register(t0);
StructuredBuffer<MinMax> AABB							: register(t1);
StructuredBuffer<IndirectCommand> InputCommands			: register(t2);
AppendStructuredBuffer<IndirectCommand> OutputCommands	: register(u0);

[RootSignature(LightCulling_PointLight_RootSig)]
[numthreads(THREAD_BLOCK_SIZE, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    // Each thread of the CS operates on one of the indirect commands.
    uint index = (groupId.x * THREAD_BLOCK_SIZE) + groupIndex;
    
    // Don't attempt to access commands that don't exist if more threads are allocated
    // than commands.
    if (index < CommandsCount)
    {
        LightDesc light = Lights[InputCommands[index].LightIndex];
        float dmin = 0;
        
        if (light.Position.x < AABB[index].Min.x) dmin += pow(light.Position.x - AABB[index].Min.x, 2); else
        if (light.Position.x > AABB[index].Max.x) dmin += pow(light.Position.x - AABB[index].Max.x, 2);
        if (light.Position.y < AABB[index].Min.y) dmin += pow(light.Position.y - AABB[index].Min.y, 2); else
        if (light.Position.y > AABB[index].Max.y) dmin += pow(light.Position.y - AABB[index].Max.y, 2);
        if (light.Position.z < AABB[index].Min.z) dmin += pow(light.Position.z - AABB[index].Min.z, 2); else
        if (light.Position.z > AABB[index].Max.z) dmin += pow(light.Position.z - AABB[index].Max.z, 2);
        
        if (dmin <= (light.Range * light.Range))
        {
            OutputCommands.Append(InputCommands[index]);
        }
    }
}