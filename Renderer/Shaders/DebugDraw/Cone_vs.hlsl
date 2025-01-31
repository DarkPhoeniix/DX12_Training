
#include "Cone_rootsig.hlsli"

struct VertexInput
{
    uint Primitive : SV_InstanceID;
};

struct GeometryInput
{
    uint Primitive : INDEX;
};

[RootSignature(Cone_RootSig)]
GeometryInput main(VertexInput input)
{
    GeometryInput output = (GeometryInput) 0;
	
    output.Primitive = input.Primitive;
	
    return output;
}