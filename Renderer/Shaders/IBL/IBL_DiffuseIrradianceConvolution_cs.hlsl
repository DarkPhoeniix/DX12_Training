
#include "IBL_DiffuseIrradianceConvolution_rootsig.hlsli"
#include "../CommonConstants.hlsli"

#define THREADS_PER_DIMENSION 8

const static float k_SampleDelta = 0.005f;

Texture2D<float4> Skybox : register(t0);
RWTexture2DArray<float4> DiffuseIrradianceMap : register(u0);

SamplerState LinearSampler : register(s0);

float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.x, v.z), asin(-v.y));
    uv *= float2(k_1_PI_2, k_1_PI);
    uv += 0.5f;
    return uv;
}

float3 GetSamplingVector(uint3 ThreadID, uint2 textureSize)
{
    float2 st = ThreadID.xy / float2(textureSize.x, textureSize.y);
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

[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
[RootSignature(IBL_RootSig)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float outputWidth, outputHeight, outputArraySize;
    DiffuseIrradianceMap.GetDimensions(outputWidth, outputHeight, outputArraySize);

    float3 N = GetSamplingVector(DTid, uint2(outputWidth, outputHeight));
    float3 up = abs(N.z) < 0.999 ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 right = normalize(cross(up, N));
    up = cross(N, right);
    
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    uint sampleCount = 0u;
    for (float phi = 0.0f; phi < k_2_PI; phi += k_SampleDelta)
    {
        for (float theta = 0.0f; theta < k_PI_2; theta += k_SampleDelta)
        {
            // Spherical to World Space in two steps...
            float3 tempVec = cos(phi) * right + sin(phi) * up;
            float3 sampleVector = normalize(cos(theta) * N + sin(theta) * tempVec);
            
            float3 sampleDir = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
        
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            
            float2 skyboxTexel = SampleSphericalMap(sampleVec);
            
            irradiance += max(0.0f, Skybox.SampleLevel(LinearSampler, skyboxTexel, 0).rgb) * cos(theta) * sin(theta); // * dot(sampleDir, N));
            sampleCount++;
        }
    }
    
    DiffuseIrradianceMap[DTid] = float4(k_PI * irradiance / float(sampleCount), 1.0f);
}