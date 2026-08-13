
#include "../UnifiedRootSignature.hlsli"
#include "../CommonConstants.hlsli"
#include "../CommonResources.hlsli"

struct GeometryInput
{
    uint Primitive : INDEX;
};

struct PixelInput
{
    float4 Position : SV_Position;
};

struct PassConstants
{
    float3 Position;
    float Radius;
    float3 Direction;
    float Height;
    float4 Color;
};

URootConstants(PassConstants, PassCB);

float3 SphericalToCartesian(float radius, float polar, float azimuth)
{
    float x = radius * sin(polar) * cos(azimuth);
    float y = radius * cos(polar);
    float z = radius * sin(polar) * sin(azimuth);
    
    return float3(x, y, z);
}

static const int NUM_SEGMENTS = 15;

[maxvertexcount(256)]
void main(point GeometryInput input[1], inout LineStream<PixelInput> lineStream)
{
    float3 up = float3(0, 1, 0); // Original cone up direction
    float3 newY = normalize(PassCB.Direction);
    float3 newX = normalize(cross(up, newY));
    float3 newZ = cross(newX, newY);

    float3x3 rotationMatrix = float3x3(newX, newY, newZ);
    
    for (int i = 0; i < NUM_SEGMENTS; ++i)
    {
        float3 positionLocal = mul(float3(0.0f, 0.0f, 0.0f), rotationMatrix);
        positionLocal += PassCB.Position;
        float4 position = mul(float4(positionLocal, 1.0f), FrameCB.ViewProjection);
        lineStream.Append((PixelInput) position);
        
        
        float angle = 2.0f * k_PI * i / NUM_SEGMENTS;
        positionLocal = float3(PassCB.Radius * sin(angle), PassCB.Height, PassCB.Radius * cos(angle));
        positionLocal = mul(positionLocal, rotationMatrix);
        positionLocal += PassCB.Position;
        position = mul(float4(positionLocal, 1.0f), FrameCB.ViewProjection);
        
        lineStream.Append((PixelInput) position);
        lineStream.RestartStrip();
        lineStream.Append((PixelInput) position);
        
        angle = 2.0f * k_PI * (i - 1) / NUM_SEGMENTS;
        positionLocal = float3(PassCB.Radius * sin(angle), PassCB.Height, PassCB.Radius * cos(angle));
        positionLocal = mul(positionLocal, rotationMatrix);
        positionLocal += PassCB.Position;
        position = mul(float4(positionLocal, 1.0f), FrameCB.ViewProjection);
        
        lineStream.Append((PixelInput) position);
        lineStream.RestartStrip();
    }
}
