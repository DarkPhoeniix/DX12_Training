
#include "../LightingCommon.hlsli"

struct GSOutput
{
	float4	Position	: SV_POSITION;
    uint	RTIndex	    : SV_RenderTargetArrayIndex;
};

struct ShadowData
{
    uint LightIndex;
};

ConstantBuffer<ShadowData> Shadow : register(b3);
StructuredBuffer<LightDesc> Lights : register(t0);

[maxvertexcount(18)]
void main(triangle float4 input[3] : SV_POSITION, inout TriangleStream<GSOutput> triangleStream)
{
    for (int faceIndex = 0; faceIndex < 6; faceIndex++)
    {
        GSOutput output;
        output.RTIndex = faceIndex;
        for (int v = 0; v < 3; v++)
        {
            output.Position = mul(input[v], Lights[Shadow.LightIndex].ViewProj[faceIndex]);
            triangleStream.Append(output);
        }
        triangleStream.RestartStrip();
    }
}