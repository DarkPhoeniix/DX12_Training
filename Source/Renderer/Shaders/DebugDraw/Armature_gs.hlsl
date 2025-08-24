
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"

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

struct BoneDesc
{
    row_major matrix Transform;
};

struct PassConstants
{
    float4 Start;
    float4 End;
    float InstanceIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

[maxvertexcount(170)]
void main(point Geometryinput input[1], inout LineStream<Pixelinput> lineStream)
{
    StructuredBuffer<ModelDesc> Instances = ResourceDescriptorHeap[FrameCB.InstancesBufferIndex];
    ModelDesc Model = Instances[PassCB.InstanceIndex];
    
    StructuredBuffer<BoneDesc> Bones = ResourceDescriptorHeap[Model.BonesBufferIndex];
    
	// for each pair of line, adding to stream
    Pixelinput psinput;
        
    float4 pos = PassCB.Start;
        
    psinput.Position = mul(pos, FrameCB.ViewProjection);
    psinput.Color = float2(1.0f, 0.0f);
    lineStream.Append(psinput);
        
    pos = PassCB.End;
        
    psinput.Position = mul(pos, FrameCB.ViewProjection);
    psinput.Color = float2(0.0f, 1.0f);
    lineStream.Append(psinput);
}