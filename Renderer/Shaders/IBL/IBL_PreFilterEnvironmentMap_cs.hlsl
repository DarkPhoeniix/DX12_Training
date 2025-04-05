
#define IBL_PreFilterRootSig \
	"RootFlags(0), " \
    "RootConstants(num32BitConstants = 1, b0, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_ALL)," \
    "DescriptorTable(UAV(u0), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \

#include "IBL_DiffuseIrradianceConvolution_rootsig.hlsli"
#include "../CommonConstants.hlsli"

#define THREADS_PER_DIMENSION 8

const static uint k_SamplesCount = 4096;

cbuffer Contants : register(b0)
{
    float Roughness;
};
Texture2D<float4> Skybox : register(t0);
RWTexture2DArray<float4> PreFilteredEnvironmentMap : register(u0);

SamplerState LinearSampler : register(s0);

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = k_PI * denom * denom;

    return nom / denom;
}

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
	
    float phi = 2.0 * k_PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space floattor to world-space sample floattor
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
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

float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.x, v.z), asin(-v.y));
    uv *= float2(k_1_PI_2, k_1_PI);
    uv += 0.5f;
    return uv;
}

[RootSignature(IBL_PreFilterRootSig)]
[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float outputWidth, outputHeight, outputArraySize;
    PreFilteredEnvironmentMap.GetDimensions(outputWidth, outputHeight, outputArraySize);

    float3 N = GetSamplingVector(DTid, uint2(outputWidth, outputHeight));
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
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;
            
            float resolution = 1024.0; // resolution of source cubemap (per face)
            float saTexel = 4.0 * k_PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(k_SamplesCount) * pdf + 0.0001);

            float mipLevel = Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
            
            float2 skyboxTexel = SampleSphericalMap(L);
            
            prefilteredColor += Skybox.SampleLevel(LinearSampler, skyboxTexel, 0).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor /= totalWeight;

    PreFilteredEnvironmentMap[DTid] = float4(prefilteredColor, 1.0);
}