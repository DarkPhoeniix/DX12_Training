
#include "../UnifiedRootSignature.hlsli"
#include "../CommonConstants.hlsli"
#include "../CommonFunctions.hlsli"

#define THREADS_PER_DIMENSION 8

struct PassConstants
{
    uint SkyboxTextureIndex;
    uint CubemapIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

SamplerState LinearSampler : register(s0);

[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
[RootSignature(URootSignature)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    TextureCube Skybox = ResourceDescriptorHeap[PassCB.SkyboxTextureIndex];
    RWTexture2DArray<float4> Cubemap = ResourceDescriptorHeap[PassCB.CubemapIndex];
    
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