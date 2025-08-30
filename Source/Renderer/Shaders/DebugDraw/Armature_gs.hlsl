
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"

struct GeometryInput
{
    uint Primitive : INDEX;
};

struct PixelInput
{
    float4 Position : SV_Position;
    float2 Color : COLOR;
};

struct PassConstants
{
    float4 Start;
    float4 End;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

[maxvertexcount(170)]
void main(point GeometryInput input[1], inout LineStream<PixelInput> lineStream)
{
	// for each pair of line, adding to stream
    PixelInput psinput;
        
    float4 pos = PassCB.Start;
        
    psinput.Position = mul(pos, FrameCB.ViewProjection);
    psinput.Color = float2(1.0f, 0.0f);
    lineStream.Append(psinput);
        
    pos = PassCB.End;
        
    psinput.Position = mul(pos, FrameCB.ViewProjection);
    psinput.Color = float2(0.0f, 1.0f);
    lineStream.Append(psinput);
}