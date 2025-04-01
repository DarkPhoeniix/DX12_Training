
#define IBL_RootSig \
	"RootFlags(0), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(UAV(u0), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    
#include "../CommonContants.hlsli"

#define THREADS_PER_DIMENSION 8

Texture2D<float4> Skybox : register(t0);
RWTexture2DArray<float4> Cubemap : register(u0);

SamplerState LinearSampler : register(s0);

float3 getSamplingVector(uint3 ThreadID)
{
    float outputWidth, outputHeight, outputDepth;
    Cubemap.GetDimensions(outputWidth, outputHeight, outputDepth);

    float2 st = ThreadID.xy / float2(outputWidth, outputHeight);
    float2 uv = 2.0 * float2(st.x, 1.0 - st.y) - float2(1.0, 1.0);

	// Select vector based on cubemap face index.
    float3 ret;
    switch (ThreadID.z)
    {
        case 0:
            ret = float3(1.0, uv.y, -uv.x);
            break;
        case 1:
            ret = float3(-1.0, uv.y, uv.x);
            break;
        case 2:
            ret = float3(uv.x, 1.0, -uv.y);
            break;
        case 3:
            ret = float3(uv.x, -1.0, uv.y);
            break;
        case 4:
            ret = float3(uv.x, uv.y, 1.0);
            break;
        case 5:
            ret = float3(-uv.x, uv.y, -1.0);
            break;
    }
    return normalize(ret);
}

float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.x, v.z), asin(-v.y));
    uv *= float2(k_1_PI_2, k_1_PI);
    uv += 0.5f;
    return uv;
}

[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
[RootSignature(IBL_RootSig)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float3 v = getSamplingVector(DTid);
	
	// Convert Cartesian direction vector to spherical coordinates.
    float phi = atan2(v.z, v.x);
    float theta = acos(v.y);

	// Sample equirectangular texture.
    float4 color = Skybox.SampleLevel(LinearSampler, SampleSphericalMap(v), 0);

	// Write out color to output cubemap.
    Cubemap[DTid] = color;
}