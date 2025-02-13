
#include "../Common.hlsli"

struct Geometryinput
{
    uint Primitive : INDEX;
};

struct Pixelinput
{
    float4 Position : SV_Position;
    float2 Color : COLOR;
};

struct ViewData
{
    row_major matrix ViewProj;
};

struct ArmatureData
{
    float4 start;
    float4 end;
};

ConstantBuffer<ViewData> Instance : register(b0);
ConstantBuffer<ArmatureData> Armature : register(b1);

StructuredBuffer<float4> BonePositions : register(t0);

[maxvertexcount(170)]
void main(point Geometryinput input[1], inout LineStream<Pixelinput> lineStream)
{
	// for each pair of line, adding to stream
    Pixelinput psinput;
        
    float4 pos = Armature.start;
        
    psinput.Position = mul(pos, Instance.ViewProj);
    psinput.Color = float2(1.0f, 0.0f);
    lineStream.Append(psinput);
        
    pos = Armature.end;
        
    psinput.Position = mul(pos, Instance.ViewProj);
    psinput.Color = float2(0.0f, 1.0f);
    lineStream.Append(psinput);
}