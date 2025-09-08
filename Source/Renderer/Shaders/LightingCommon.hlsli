#pragma once

#define LIGHT_TYPE_DIRECTIONAL  0
#define LIGHT_TYPE_POINT        1
#define LIGHT_TYPE_SPOT         2

#define CONSTANT_SHADOW_BIAS    0.0002f
#define SLOPE_SHADOW_BIAS       0.00005f

struct Surface          // 164 bytes
{
    float4              NDCPosition;
    float4              Position;
    
    float3              Albedo;
    float               Metallic;
    float3              Normal;
    float               Roughness;
    float3              Emission;
    
    float4              FinalColor;
    
    float4              ViewDirection;
    float4              ToLight;
    float4              Reflect;
    
    float               DistanceToL;
    float               NdotV;
    float               NdotL;
    float               NdotH;
};

struct LightDesc        // 480 bytes
{
    float4              Direction;
    float4              Position;
    float4              Color;
    
    float               Intesity;
    float               Range;
    float               OuterAngle;
    float               InnerAngle;
    
    uint                Type;
    uint                CastShadows;
    
    float2              PerspectiveValues;
    row_major matrix    ViewProj[6];
    
    uint                ShadowMapIndex;
    
    uint                pad[3];
};

float CalculatePointLightAttenuation(in LightDesc light, in Surface surface)
{
    return saturate(((surface.DistanceToL * surface.DistanceToL) / (light.Range * light.Range)) * (((2 * surface.DistanceToL) / light.Range) - 3.0f) + 1.0f);
}

float CalculateSpotLightAttenuation(in LightDesc light, in Surface surface)
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

float CalculateAttenuation(in LightDesc light, in Surface surface)
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

float CalculatePointLightShadowAttenuation(in TextureCube texture, in SamplerComparisonState cmpSampler, in LightDesc light, in Surface surface)
{
    row_major matrix VP = light.ViewProj[0];
    float4 surfacePos = mul(surface.Position, VP);
    surfacePos /= surfacePos.w;
    
    float3 UVD;
    UVD.x = (surfacePos.x * 0.5f) + 0.5f;
    UVD.y = (surfacePos.y * -0.5f) + 0.5f;
    UVD.z = surfacePos.z - 0.001f;
    
    float shadowFactor = 0.0f;
    
    float3 location = surface.Position.xyz - light.Position.xyz;
    float3 absLocation = abs(location);
    float Z = max(absLocation.x, max(absLocation.y, absLocation.z));
    float Depth = (light.PerspectiveValues[0] * Z + light.PerspectiveValues[1]) / Z;
    
    float bias = CONSTANT_SHADOW_BIAS + SLOPE_SHADOW_BIAS * sqrt(1 - surface.NdotL * surface.NdotL) / surface.NdotL;
    
    shadowFactor = texture.SampleCmpLevelZero(cmpSampler, location, Depth - bias);
    
    return shadowFactor;
}

float CalculateSpotLightShadowAttenuation(in Texture2D texture, in SamplerComparisonState cmpSampler, in LightDesc light, in Surface surface)
{
    row_major matrix VP = light.ViewProj[0];
    float4 surfacePos = mul(surface.Position, VP);
    surfacePos /= surfacePos.w;
    
    float3 UVD;
    UVD.x = (surfacePos.x * 0.5f) + 0.5f;
    UVD.y = (surfacePos.y * -0.5f) + 0.5f;
    UVD.z = surfacePos.z - 0.001f;
    
    float shadowFactor = 0.0f;
    float bias = CONSTANT_SHADOW_BIAS + SLOPE_SHADOW_BIAS * tan(acos(surface.NdotL));
    
    shadowFactor = texture.SampleCmpLevelZero(cmpSampler, UVD.xy, (UVD.z - bias));
    
    return shadowFactor;
}
