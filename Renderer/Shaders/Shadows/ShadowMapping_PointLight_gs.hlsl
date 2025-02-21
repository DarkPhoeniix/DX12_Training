
#include "../LightingCommon.hlsli"

struct GSOutput
{
	float4	Position	: SV_POSITION;
    uint	RTIndex	    : SV_RenderTargetArrayIndex;
};

cbuffer Light : register(b3)
{
    uint LightIndex : packoffset(c0.x);
}
StructuredBuffer<LightDesc> Lights : register(t2);

[maxvertexcount(18)]
void main(triangle float4 input[3] : SV_POSITION, inout TriangleStream<GSOutput> triangleStream)
{
    for (int faceIndex = 0; faceIndex < 6; faceIndex++)
    {
        GSOutput output;
        output.RTIndex = faceIndex;
        for (int v = 0; v < 3; v++)
        {
            output.Position = mul(input[v], Lights[LightIndex].ViewProj[faceIndex]);
            triangleStream.Append(output);
        }
        triangleStream.RestartStrip();
    }
}