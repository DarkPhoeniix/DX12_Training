#pragma once

struct Surface
{
    float4 Position;
    
    float4 Albedo;
    float4 Normal;
    float Metalness;
    float Roughness;
    
    float4 FinalColor;
    
    float DistanceToL;
    float NdotV;
    float NdotL;
    float NdotH;
};

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1

struct LightDesc
{
    float4 Direction;
    float4 Position;
    float4 Color;
    
    float Intesity;
    float Range;
    
    uint Type;
    
    uint pad;
};

float CalculateInverseSquareAttenuation(LightDesc light, Surface surface)
{
    if (light.Type == LIGHT_TYPE_DIRECTIONAL)
    {
        return 1.0f;
    }
    else if (light.Type == LIGHT_TYPE_POINT)
    {
        return saturate(((surface.DistanceToL * surface.DistanceToL) / (light.Range * light.Range)) * (((2 * surface.DistanceToL) / light.Range) - 3.0f) + 1.0f);
    }
    else
    {
        return 0.0f;
    }
}
