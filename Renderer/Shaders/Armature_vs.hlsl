
#include "Armature_rootsig.hlsli"

struct VertexInput
{
    uint primitive : SV_InstanceID;
};

struct GeometryInput
{
    uint primitive : INDEX;
};

[RootSignature(Armature_RootSig)]
GeometryInput main(VertexInput input)
{
    GeometryInput output;
    output.primitive = input.primitive;
	
    return output;
}