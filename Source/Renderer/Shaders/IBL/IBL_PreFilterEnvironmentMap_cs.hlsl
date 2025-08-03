
#include "../UnifiedRootSignature.hlsli"
#include "../CommonConstants.hlsli"
#include "../CommonFunctions.hlsli"
#include "IBL_Helpers.hlsli"

#define THREADS_PER_DIMENSION 8

struct PassContants
{
    float Roughness;
    
    uint SkyboxTextureIndex;
    uint PreFilteredEnvironmentMapIndex;
};

ConstantBuffer<PassContants> PassCB : register(b1);

const static uint k_SamplesCount = 4096u;

[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
[RootSignature(URootSignature)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Texture2D<float4> Skybox = ResourceDescriptorHeap[PassCB.SkyboxTextureIndex];
    RWTexture2DArray<float4> PreFilteredEnvironmentMap = ResourceDescriptorHeap[PassCB.PreFilteredEnvironmentMapIndex];
    
    float outputWidth, outputHeight, outputArraySize;
    PreFilteredEnvironmentMap.GetDimensions(outputWidth, outputHeight, outputArraySize);

    float3 N = SampleTextureArrayAsCube(DTid, uint2(outputWidth, outputHeight));
    float3 R = N;
    float3 V = R;
    
    float totalWeight = 0.0;
    float3 prefilteredColor = float3(0.0, 0.0, 0.0);
    for (uint i = 0u; i < k_SamplesCount; ++i)
    {
        float2 Xi = Hammersley(i, k_SamplesCount);
        float3 H = ImportanceSampleGGX(Xi, N, PassCB.Roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D = DistributionGGX(N, H, PassCB.Roughness);
            float NdotH = max(dot(N, H), 0.0f);
            float HdotV = max(dot(H, V), 0.0f);
            float pdf = D * NdotH / (4.0f * HdotV) + 0.0001f;
            
            float resolution = 1024.0f; // resolution of source cubemap (per face)
            float saTexel = 4.0f * k_PI / (6.0f * resolution * resolution);
            float saSample = 1.0f / (float(k_SamplesCount) * pdf + 0.0001f);

            float mipLevel = (PassCB.Roughness == 0.0f) ? 0.0f : 0.5f * log2(saSample / saTexel);
            
            float2 skyboxTexel = SampleSphericalMap(L);
            
            // clamp upper value to 12 to avoid convolution visual artifacts
            float3 value = min(float3(12.0f, 12.0f, 12.0f), max(0.0f, Skybox.SampleLevel(LinearClampSampler, skyboxTexel, 0).rgb));
            prefilteredColor += value * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor /= totalWeight;

    PreFilteredEnvironmentMap[DTid] = float4(prefilteredColor, 1.0f);
}