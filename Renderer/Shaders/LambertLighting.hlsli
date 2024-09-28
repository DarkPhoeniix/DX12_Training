
#include "LightingCommon.hlsli"

#define AMBIENT_IMPACT 0.1f

float4 CalculateAmbient(in Surface surface)
{
    return surface.Albedo * AMBIENT_IMPACT;
}

float4 CalculateDiffuse(in Surface surface, in LightDesc light)
{
    float diffuseFactor = 0.0f;
    
    if (light.Type == LIGHT_TYPE_DIRECTIONAL)
    {
        diffuseFactor = max(dot(surface.Normal, -light.Direction), 0.0f);
    }
    else if (light.Type == LIGHT_TYPE_POINT)
    {
        float4 lightDirection = normalize(light.Position - surface.Positon);
        diffuseFactor = max(dot(surface.Normal, lightDirection), 0.0f);
    }
    
    return surface.Albedo * light.Color * diffuseFactor;
}
