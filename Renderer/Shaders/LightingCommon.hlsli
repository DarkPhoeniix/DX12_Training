#pragma once

struct Surface
{
    float4 NDCPosition;
    float4 Position;
    
    float4 Albedo;
    float4 Normal;
    float Metalness;
    float Roughness;
    
    float4 FinalColor;
    
    float4 ToLight;
    float DistanceToL;
    float NdotV;
    float NdotL;
    float NdotH;
};

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct LightDesc
{
    float4 Direction;
    float4 Position;
    float4 Color;
    
    float Intesity;
    float Range;
    float OuterAngle;
    float InnerAngle;
    
    uint Type;
    uint CastShadows;
    
    float2 PerspectiveValues;
    row_major matrix ViewProj[6];
    
    uint ShadowMapIndex;
    
    uint pad[3];
};

float CalculatePointLightAttenuation(LightDesc light, Surface surface)
{
    return saturate(((surface.DistanceToL * surface.DistanceToL) / (light.Range * light.Range)) * (((2 * surface.DistanceToL) / light.Range) - 3.0f) + 1.0f);
}

float CalculateSpotLightAttenuation(LightDesc light, Surface surface)
{
    float3 toLight = normalize(light.Position - surface.Position);
    float cosAngle = dot(-light.Direction.xyz, toLight);
    float cosOuterAngle = light.OuterAngle;
    float cosInnerAngle = light.InnerAngle;
    
    float coneAttenuation = saturate((cosAngle - cosOuterAngle) / (cosInnerAngle - cosOuterAngle));
    coneAttenuation *= coneAttenuation;
    
    float rangeRcp = 1.0f / light.Range;
    float distanceToLightNorm = 1.0f - saturate(surface.DistanceToL * rangeRcp);
    
    float attenuation = distanceToLightNorm * distanceToLightNorm;

    return attenuation * coneAttenuation;
}

float CalculateAttenuation(LightDesc light, Surface surface)
{
    if (light.Type == LIGHT_TYPE_DIRECTIONAL)
    {
        return 1.0f;
    }
    else if (light.Type == LIGHT_TYPE_POINT)
    {
        return CalculatePointLightAttenuation(light, surface);
    }
    else if (light.Type == LIGHT_TYPE_SPOT)
    {
        return CalculateSpotLightAttenuation(light, surface);
    }
    
    return 0.0f;
}

uint GetCubeFaceIndex(float3 toPixel)
{
    float3 vAbs = abs(toPixel);
    uint faceIndex = 0;
    if (vAbs.z >= vAbs.x && vAbs.z >= vAbs.y)
    {
        faceIndex = toPixel.z < 0 ? 5 : 4;
    }
    else if (vAbs.y >= vAbs.x)
    {
        faceIndex = toPixel.y < 0 ? 3 : 2;
    }
    else
    {
        faceIndex = toPixel.x < 0 ? 1 : 0;
    }
    
    return faceIndex;
}
