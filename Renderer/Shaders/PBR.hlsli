
#include "Common.hlsli"
#include "LightingCommon.hlsli"

// Trowbridge-Reitz GGX normal distribution function
float CalculateSpecular(in Surface surface, in LightDesc light)
{
    float3 lightDirection;
    if (light.Type == LIGHT_TYPE_DIRECTIONAL)
    {
        lightDirection = -normalize(light.Direction);
    }
    else if (light.Type == LIGHT_TYPE_POINT)
    {
        lightDirection = normalize(light.Position - surface.Positon);
    }
    float3 viewDirection = transpose(Scene.View)[2].xyz;
    float3 halfway = normalize(viewDirection + lightDirection);
    
    float a2 = surface.Roughness * surface.Roughness;
    float NdotH = max(dot(surface.Normal.xyz, halfway), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float nominator = a2;
    float denominator = (NdotH2 * (a2 - 1) + 1);
    denominator = 3.141592f * denominator * denominator;
    
    return nominator / denominator;
}

// Schlick-GGX geometry function
float GeometrySchlickGGX(in float NdotV, in float k)
{
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

float GeometrySmith(in Surface surface, in LightDesc light)
{
    float3 lightDirection;
    if (light.Type == LIGHT_TYPE_DIRECTIONAL)
    {
        lightDirection = -normalize(light.Direction);
    }
    else if (light.Type == LIGHT_TYPE_POINT)
    {
        lightDirection = normalize(light.Position - surface.Positon);
    }
    float3 viewDirection = transpose(Scene.View)[2].xyz;
    
    float NdotV = max(dot(surface.Normal.xyz, viewDirection), 0.0f);
    float NdotL = max(dot(surface.Normal.xyz, lightDirection), 0.0f);
    
    float k = (surface.Roughness + 1) * (surface.Roughness + 1) / 8.0f;
    
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}