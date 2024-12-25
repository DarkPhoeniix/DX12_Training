
#include "OBB_rootsig.hlsli"

struct VertexInput
{
    uint primitive : SV_InstanceID;
};

struct GeometryInput
{
    uint primitive : INDEX;
};

[RootSignature(OBB_RootSig)]
GeometryInput main(VertexInput input)
{
    GeometryInput output = (GeometryInput) 0;
	
    output.primitive = input.primitive;
	
    return output;
}