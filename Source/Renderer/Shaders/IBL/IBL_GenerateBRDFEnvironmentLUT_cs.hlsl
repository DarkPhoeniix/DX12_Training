
#include "../UnifiedRootSignature.hlsli"
#include "../CommonConstants.hlsli"
#include "IBL_Helpers.hlsli"

#define THREADS_PER_DIMENSION 8

struct PassConstants
{
    uint brdfLUTTextureIndex;
};

URootConstants(PassConstants, PassCB);

const static uint k_SamplesCount = 1024u;

float2 IntegrateBRDF(float NdotV, float roughness, uint sampleCount)
{
    float3 V;
    V.x = sqrt(1.0f - NdotV * NdotV);
    V.y = 0.0f;
    V.z = NdotV;

    float A = 0.0f;
    float B = 0.0f;

    float3 N = float3(0.0f, 0.0f, 1.0f);
    
    for (uint i = 0u; i < sampleCount; ++i)
    {
        // generates a sample floattor that's biased towards the
        // preferred alignment direction (importance sampling).
        float2 Xi = Hammersley(i, sampleCount);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0f);
        float NdotH = max(H.z, 0.0f);
        float VdotH = max(dot(V, H), 0.0f);

        if (NdotL > 0.0f)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    
    A /= float(sampleCount);
    B /= float(sampleCount);
    
    return float2(A, B);
}

[RootSignature(URootSignature)]
[numthreads(THREADS_PER_DIMENSION, THREADS_PER_DIMENSION, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    RWTexture2DArray<float2> BRDF_LUT = ResourceDescriptorHeap[PassCB.brdfLUTTextureIndex];
    
    float x, y, z;
    BRDF_LUT.GetDimensions(x, y, z);
    
    if (DTid.x >= x || DTid.y >= y)
    {
        return;
    }
    
    float NdotV = max(0.001f, DTid.x / (x - 1.0f));
    float roughness = DTid.y / (y - 1.0f);
    
    BRDF_LUT[DTid] = IntegrateBRDF(NdotV, roughness, k_SamplesCount);
}
