
#include "Common.hlsli"
#include "CommonConstants.hlsli"
#include "LightingCommon.hlsli"

// Trowbridge-Reitz GGX normal distribution function (D)
float CalculateSpecular(in Surface surface)
{    
    float a2 = max(0.001f, surface.Roughness * surface.Roughness);
    float NdotH2 = surface.NdotH * surface.NdotH;
    
    float nominator = a2;
    float denominator = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denominator = k_PI * denominator * denominator;
    
    return nominator / denominator;
}

// Schlick-GGX geometry function (G)
float GeometrySchlickGGX(in Surface surface, in float k)
{
    float nom = surface.NdotV;
    float denom = surface.NdotV * (1.0f - k) + k;
	
    return nom / denom;
}

float GeometrySmith(in Surface surface)
{
    float k = (surface.Roughness + 1.0f) * (surface.Roughness + 1.0f) / 8.0f;
    
    float ggx1 = GeometrySchlickGGX(surface, k);
    float ggx2 = GeometrySchlickGGX(surface, k);
	
    return ggx1 * ggx2;
}

// Fresnel function (F)
float3 FresnelSchlick(in Surface surface, in float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - surface.NdotH, 5.0f);
}
