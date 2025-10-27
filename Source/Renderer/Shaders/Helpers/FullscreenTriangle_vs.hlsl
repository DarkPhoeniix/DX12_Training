
#include "../UnifiedRootSignature.hlsli"

struct VSOutput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

[RootSignature(URootSignature)]
VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    output.UV = float2((id << 1) & 2, id & 2);
    output.Position = float4(output.UV * float2(2, -2) + float2(-1, 1), 0, 1);
    
    return output;
}
