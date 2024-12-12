
#include "Common.hlsli"

struct GeometryInput
{
    uint Primitive  : INDEX;
};

struct PixelInput
{
    float4 Position : SV_Position;
    float2 Color    : COLOR;
};

struct ViewData
{
    row_major matrix ViewProj;
};

struct ArmatureData
{
    uint Size;
};

ConstantBuffer<ViewData> Instance : register(b0);
ConstantBuffer<ArmatureData> Armature : register(b1);

StructuredBuffer<float4> BonePositions : register(t0);

[maxvertexcount(170)]
void main(point GeometryInput input[1], inout LineStream<PixelInput> lineStream)
{
    uint size = Armature.Size;
    
	// for each pair of line, adding to stream
    for (uint i = 0; i < size; i += 2)
    {
        PixelInput psInput;
        
        float4 pos = BonePositions[i];
        
        psInput.Position = mul(pos, Instance.ViewProj);
        psInput.Color = float2(1.0f, 0.0f);
        lineStream.Append(psInput);
        
        pos = BonePositions[i + 1];
        
        psInput.Position = mul(pos, Instance.ViewProj);
        psInput.Color = float2(0.0f, 1.0f);
        lineStream.Append(psInput);
        
        lineStream.RestartStrip();
    }
}