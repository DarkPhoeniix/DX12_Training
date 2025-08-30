
#include "../UnifiedRootSignature.hlsli"
#include "../CommonConstants.hlsli"
#include "../CommonResources.hlsli"

struct Geometryinput
{
    uint Primitive : INDEX;
};

struct Pixelinput
{
    float4 Position : SV_Position;
};

struct PassConstants
{
    float3 Position;
    float Radius;
    float4 Color;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

float3 SphericalToCartesian(float radius, float polar, float azimuth)
{
    float x = radius * sin(polar) * cos(azimuth);
    float y = radius * cos(polar);
    float z = radius * sin(polar) * sin(azimuth);
    
    return float3(x, y, z);
}

static const int NUM_SEGMENTS = 32;

[maxvertexcount(256)]
void main(point Geometryinput input[1], inout LineStream<Pixelinput> lineStream)
{
    [unroll(NUM_SEGMENTS)]
    for (int i = 0; i < NUM_SEGMENTS; ++i)
    {
        float polar = (2.0f * k_PI * i) / NUM_SEGMENTS;
        float3 position = float3(PassCB.Radius * sin(polar), 0.0f, PassCB.Radius * cos(polar)) + PassCB.Position;
        float4 worldPosition = mul(float4(position, 1.0f), FrameCB.ViewProjection);
            
        lineStream.Append((Pixelinput) worldPosition);
        
        polar = (2.0f * k_PI * (i - 1)) / NUM_SEGMENTS;
        position = float3(PassCB.Radius * sin(polar), 0.0f, PassCB.Radius * cos(polar)) + PassCB.Position;
        worldPosition = mul(float4(position, 1.0f), FrameCB.ViewProjection);
            
        lineStream.Append((Pixelinput) worldPosition);
        lineStream.RestartStrip();
        
        polar = (2.0f * k_PI * i) / NUM_SEGMENTS;
        position = float3(0.0f, PassCB.Radius * sin(polar), PassCB.Radius * cos(polar)) + PassCB.Position;
        worldPosition = mul(float4(position, 1.0f), FrameCB.ViewProjection);
            
        lineStream.Append((Pixelinput) worldPosition);
        
        polar = (2.0f * k_PI * (i - 1)) / NUM_SEGMENTS;
        position = float3(0.0f, PassCB.Radius * sin(polar), PassCB.Radius * cos(polar)) + PassCB.Position;
        worldPosition = mul(float4(position, 1.0f), FrameCB.ViewProjection);
            
        lineStream.Append((Pixelinput) worldPosition);
        lineStream.RestartStrip();
        
        polar = (2.0f * k_PI * i) / NUM_SEGMENTS;
        position = float3(PassCB.Radius * sin(polar), PassCB.Radius * cos(polar), 0.0f) + PassCB.Position;
        worldPosition = mul(float4(position, 1.0f), FrameCB.ViewProjection);
            
        lineStream.Append((Pixelinput) worldPosition);
        
        polar = (2.0f * k_PI * (i - 1)) / NUM_SEGMENTS;
        position = float3(PassCB.Radius * sin(polar), PassCB.Radius * cos(polar), 0.0f) + PassCB.Position;
        worldPosition = mul(float4(position, 1.0f), FrameCB.ViewProjection);
            
        lineStream.Append((Pixelinput) worldPosition);
        lineStream.RestartStrip();
    }
}
