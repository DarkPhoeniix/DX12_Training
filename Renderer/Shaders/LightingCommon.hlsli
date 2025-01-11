#pragma once

struct Surface
{
    float4 Positon;
    
    float4 Albedo;
    float4 Normal;
    float Metalness;
    float Roughness;
    
    float4 FinalColor;
    
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
};
