
#include "OBB_rootsig.hlsli"

struct Vertexinput
{
    uint primitive : SV_InstanceID;
};

struct Geometryinput
{
    uint primitive : INDEX;
};

[RootSignature(OBB_RootSig)]
Geometryinput main(Vertexinput input)
{
    Geometryinput output = (Geometryinput) 0;
	
    output.primitive = input.primitive;
	
    return output;
}