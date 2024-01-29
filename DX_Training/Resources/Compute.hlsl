
// The Vertex Shader (VS) stage is responsible for transforming the vertex data 
// from object-space into clip-space, performing (skeletal) animation or computing 
// per-vertex lighting.

#define Sprite_RootSig \
	"RootFlags " \
	"( " \
		"ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | " \
		"DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS " \
	"), " \
	"SRV(t0, visibility=SHADER_VISIBILITY_ALL), " \
    "DescriptorTable( UAV(u0, flags = DESCRIPTORS_VOLATILE) )"\

struct _Constants
{
    float4 colorStart;
    float4 colorEnd;
};

StructuredBuffer<_Constants> Constants : register(t0);

struct _Output
{
    float res;
};

RWTexture2D<float4> Output : register(u0);

#define numGroups 8

[RootSignature(Sprite_RootSig)]
[numthreads(numGroups, numGroups, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    float alpha = (float) DTid.x / 64.0f;
    alpha = saturate(alpha);
    Output[DTid.xy] = lerp(Constants[0].colorStart, Constants[0].colorEnd, alpha);
}
