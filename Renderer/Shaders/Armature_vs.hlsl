
#include "Armature_rootsig.hlsli"

struct Vertexinput
{
    uint primitive : SV_InstanceID;
};

struct Geometryinput
{
    uint primitive : INDEX;
};

[RootSignature(Armature_RootSig)]
Geometryinput main(Vertexinput input)
{
    Geometryinput output;
    output.primitive = input.primitive;
	
    return output;
}