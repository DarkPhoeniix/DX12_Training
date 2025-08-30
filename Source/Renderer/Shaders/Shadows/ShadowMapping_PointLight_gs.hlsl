
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../LightingCommon.hlsli"

struct GSOutput
{
	float4	Position	: SV_POSITION;
    uint	RTIndex	    : SV_RenderTargetArrayIndex;
};

struct PassConstants
{
    uint InstanceIndex;
    uint LightIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

[maxvertexcount(18)]
void main(triangle float4 input[3] : SV_POSITION, inout TriangleStream<GSOutput> triangleStream)
{
    StructuredBuffer<LightDesc> LightsBuffer = ResourceDescriptorHeap[FrameCB.LightsBufferIndex];
    LightDesc light = LightsBuffer[PassCB.LightIndex];
    
    for (int faceIndex = 0; faceIndex < 6; faceIndex++)
    {
        GSOutput output;
        output.RTIndex = faceIndex;
        for (int v = 0; v < 3; v++)
        {
            output.Position = mul(input[v], light.ViewProj[faceIndex]);
            triangleStream.Append(output);
        }
        triangleStream.RestartStrip();
    }
}