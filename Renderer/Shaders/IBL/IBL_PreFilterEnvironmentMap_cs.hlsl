
#define IBL_PreFilterEnvironment_RootSig \
	"RootFlags(0), " \
    "RootConstants(num32BitConstants = 1, b0, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(UAV(u0), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \

#include "../CommonConstants.hlsli"
#include "../CommonFunctions.hlsli"
#include "IBL_Helpers.hlsli"

#define THREADS_PER_DIMENSION 8

cbuffer Contants                                    : register(b0)
{
    float Roughness;
};
Texture2D<float4> Skybox                            : register(t0);
RWTexture2DArray<float4> PreFilteredEnvironmentMap  : register(u0);

SamplerState LinearSampler                          : register(s0);

const static uint k_SamplesCount = 4096u;

[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
[RootSignature(IBL_PreFilterEnvironment_RootSig)]
void main(uint3 DTid : SV_DispatchThreadID)
{
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
        float3 H = ImportanceSampleGGX(Xi, N, Roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D = DistributionGGX(N, H, Roughness);
            float NdotH = max(dot(N, H), 0.0f);
            float HdotV = max(dot(H, V), 0.0f);
            float pdf = D * NdotH / (4.0f * HdotV) + 0.0001f;
            
            float resolution = 1024.0f; // resolution of source cubemap (per face)
            float saTexel = 4.0f * k_PI / (6.0f * resolution * resolution);
            float saSample = 1.0f / (float(k_SamplesCount) * pdf + 0.0001f);

            float mipLevel = (Roughness == 0.0f) ? 0.0f : 0.5f * log2(saSample / saTexel);
            
            float2 skyboxTexel = SampleSphericalMap(L);
            
            prefilteredColor += Skybox.SampleLevel(LinearSampler, skyboxTexel, 0.0f).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor /= totalWeight;

    PreFilteredEnvironmentMap[DTid] = float4(prefilteredColor, 1.0f);
}