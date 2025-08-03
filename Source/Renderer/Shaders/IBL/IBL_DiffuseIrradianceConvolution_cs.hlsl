
#include "../UnifiedRootSignature.hlsli"
#include "../CommonConstants.hlsli"
#include "../CommonFunctions.hlsli"

#define THREADS_PER_DIMENSION 8

struct PassConstants
{
    uint SkyboxTextureIndex;
    uint DiffuseIrradianceMapIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

const static float k_SampleDelta = 0.005f;

[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
[RootSignature(URootSignature)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Texture2D<float4> Skybox = ResourceDescriptorHeap[PassCB.SkyboxTextureIndex];
    RWTexture2DArray<float4> DiffuseIrradianceMap = ResourceDescriptorHeap[PassCB.DiffuseIrradianceMapIndex];
    
    float outputWidth, outputHeight, outputArraySize;
    DiffuseIrradianceMap.GetDimensions(outputWidth, outputHeight, outputArraySize);

    float3 N = SampleTextureArrayAsCube(DTid, uint2(outputWidth, outputHeight));
    float3 up = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 right = normalize(cross(up, N));
    up = cross(N, right);
    
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    uint sampleCount = 0u;
    for (float phi = 0.0f; phi < k_2_PI; phi += k_SampleDelta)
    {
        for (float theta = 0.0f; theta < k_PI_2; theta += k_SampleDelta)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            float2 skyboxTexel = SampleSphericalMap(sampleVec);
            
            // clamp upper value to 12 to avoid convolution visual artifacts
            float3 value = min(float3(12.0f, 12.0f, 12.0f), max(0.0f, Skybox.SampleLevel(LinearClampSampler, skyboxTexel, 0).rgb));
            irradiance +=  value * cos(theta) * sin(theta);
            sampleCount++;
        }
    }
    
    DiffuseIrradianceMap[DTid] = float4(k_PI * irradiance / float(sampleCount), 1.0f);
}