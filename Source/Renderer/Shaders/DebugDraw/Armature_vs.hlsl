
#include "../UnifiedRootSignature.hlsli"

struct Vertexinput
{
    uint primitive : SV_InstanceID;
};

struct Geometryinput
{
    uint primitive : INDEX;
};

[RootSignature(URootSignature)]
Geometryinput main(Vertexinput input)
{
    Geometryinput output;
    output.primitive = input.primitive;
	
    return output;
}