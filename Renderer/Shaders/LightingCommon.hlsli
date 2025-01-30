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
    
    row_major matrix ViewProj;
    
    uint Type;
    
    bool CastShadows;
    uint ShadowMapIndex;
    
    uint pad;
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

float CalculateShadowAttenuation_PCF3x3(Texture2D shadowMap, SamplerComparisonState shadowSampler, LightDesc light, Surface surface)
{
    float4 surfacePos = mul(surface.Position, light.ViewProj);
    surfacePos /= surfacePos.w;
    
    if (surfacePos.x < -1.0f || surfacePos.x > 1.0f ||
        surfacePos.y < -1.0f || surfacePos.y > 1.0f ||
        surfacePos.z < 0.0f || surfacePos.z > 1.0f)
        return 0.0f;
    
    float3 UVD;
    UVD.x = (surfacePos.x / 2.0f) + 0.5f;
    UVD.y = (surfacePos.y / -2.0f) + 0.5f;
    UVD.z = surfacePos.z - 0.001f;
    
    float2 offsets[9] =
    {
        float2(-1, -1), float2(-1, 0), float2(-1, 1),
        float2(0, -1), float2(0, 0), float2(0, 1),
        float2(1, -1), float2(1, 0), float2(1, 1)
    };
    
    float shadowFactor = 0.0f;
    
    [unroll(9)]
    for (uint i = 0; i < 9; ++i)
    {
        shadowFactor += shadowMap.SampleCmpLevelZero(shadowSampler, UVD.xy, UVD.z, offsets[i]);
    }
    shadowFactor /= 9.0f;
    
    return shadowFactor;
}
