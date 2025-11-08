
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"

#define THREAD_BLOCK_SIZE 16

struct IndirectCommand
{
    uint2 VertexBufferAddress;
    uint  VertexBufferSize;
    uint  VertexBufferStride;   // 16
    
    uint2 SkinBufferAddress;
    uint  SkinBufferSize;
    uint  SkinBufferStride;     // 32
    
    uint2 IndexBufferAddress;
    uint IndexBufferSize;
    uint IndexBufferStride;     // 48
    
    uint2 FrameBufferAddress;
    uint  InstanceIndex;
    uint  LightIndex;           // 64
    
    uint DrawArguments[5];      // 84
    uint pad[3];                // 96
};

struct MinMax
{
    float4 Min;
    float4 Max;
};

struct PassConstants
{
    uint LightIndex;
    uint CommandsCount;
    
    uint AABBBufferIndex;
    uint InputCommandsBufferIndex;
    uint OutputCommandsBufferIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

[RootSignature(URootSignature)]
[numthreads(THREAD_BLOCK_SIZE, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    StructuredBuffer<LightDesc> Lights                      = ResourceDescriptorHeap[FrameCB.LightsBufferIndex];
    StructuredBuffer<MinMax> AABB                           = ResourceDescriptorHeap[PassCB.AABBBufferIndex];
    StructuredBuffer<IndirectCommand> InputCommands         = ResourceDescriptorHeap[PassCB.InputCommandsBufferIndex];
    AppendStructuredBuffer<IndirectCommand> OutputCommands  = ResourceDescriptorHeap[PassCB.OutputCommandsBufferIndex];
    
    // Each thread of the CS operates on one of the indirect commands.
    uint index = (groupId.x * THREAD_BLOCK_SIZE) + groupIndex;
    
    // Don't attempt to access commands that don't exist if more threads are allocated
    // than commands.
    if (index < PassCB.CommandsCount)
    {
        LightDesc light = Lights[PassCB.LightIndex];
        float dmin = 0;
        
        if (light.Position.x < AABB[index].Min.x) dmin += pow(light.Position.x - AABB[index].Min.x, 2); else
        if (light.Position.x > AABB[index].Max.x) dmin += pow(light.Position.x - AABB[index].Max.x, 2);
        if (light.Position.y < AABB[index].Min.y) dmin += pow(light.Position.y - AABB[index].Min.y, 2); else
        if (light.Position.y > AABB[index].Max.y) dmin += pow(light.Position.y - AABB[index].Max.y, 2);
        if (light.Position.z < AABB[index].Min.z) dmin += pow(light.Position.z - AABB[index].Min.z, 2); else
        if (light.Position.z > AABB[index].Max.z) dmin += pow(light.Position.z - AABB[index].Max.z, 2);
        
        if (dmin <= (light.Range * light.Range))
        {
            IndirectCommand command = InputCommands[index];
            OutputCommands.Append(command);
        }
    }
}