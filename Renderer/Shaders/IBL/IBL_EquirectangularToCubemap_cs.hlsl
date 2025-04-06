
#define IBL_EquirectangularToCubemap_RootSig \
	"RootFlags(0), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(UAV(u0), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    
#include "../CommonConstants.hlsli"
#include "../CommonFunctions.hlsli"

#define THREADS_PER_DIMENSION 8

TextureCube Skybox                  : register(t0);
RWTexture2DArray<float4> Cubemap    : register(u0);

SamplerState LinearSampler : register(s0);

[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
[RootSignature(IBL_EquirectangularToCubemap_RootSig)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float outputWidth, outputHeight, outputDepth;
    Cubemap.GetDimensions(outputWidth, outputHeight, outputDepth);

    float3 direction = SampleTextureArrayAsCube(DTid, uint2(outputWidth, outputHeight));
	
	// Convert Cartesian direction vector to spherical coordinates.
    float phi   = atan2(direction.z, direction.x);
    float theta = acos(direction.y);

	// Sample equirectangular texture.
    float4 color = Skybox.SampleLevel(LinearSampler, direction, 0);

	// Write out color to output cubemap.
    Cubemap[DTid] = color;
}