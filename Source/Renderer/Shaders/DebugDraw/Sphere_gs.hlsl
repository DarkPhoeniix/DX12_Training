
#define PI 3.14159265359

struct Geometryinput
{
    uint Primitive : INDEX;
};

struct Pixelinput
{
    float4 Position : SV_Position;
};

struct Camera
{
    row_major matrix ViewProj;
};

struct Sphere
{
    float3 Position;
    float Radius;
};

ConstantBuffer<Camera> Instance : register(b0);
ConstantBuffer<Sphere> SphereData : register(b1);

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
        float polar = (2.0f * PI * i) / NUM_SEGMENTS;
        float3 position = float3(SphereData.Radius * sin(polar), 0.0f, SphereData.Radius * cos(polar)) + SphereData.Position;
        float4 worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
        lineStream.Append((Pixelinput) worldPosition);
        
        polar = (2.0f * PI * (i - 1)) / NUM_SEGMENTS;
        position = float3(SphereData.Radius * sin(polar), 0.0f, SphereData.Radius * cos(polar)) + SphereData.Position;
        worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
        lineStream.Append((Pixelinput) worldPosition);
        lineStream.RestartStrip();
        
        polar = (2.0f * PI * i) / NUM_SEGMENTS;
        position = float3(0.0f, SphereData.Radius * sin(polar), SphereData.Radius * cos(polar)) + SphereData.Position;
        worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
        lineStream.Append((Pixelinput) worldPosition);
        
        polar = (2.0f * PI * (i - 1)) / NUM_SEGMENTS;
        position = float3(0.0f, SphereData.Radius * sin(polar), SphereData.Radius * cos(polar)) + SphereData.Position;
        worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
        lineStream.Append((Pixelinput) worldPosition);
        lineStream.RestartStrip();
        
        polar = (2.0f * PI * i) / NUM_SEGMENTS;
        position = float3(SphereData.Radius * sin(polar), SphereData.Radius * cos(polar), 0.0f) + SphereData.Position;
        worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
        lineStream.Append((Pixelinput) worldPosition);
        
        polar = (2.0f * PI * (i - 1)) / NUM_SEGMENTS;
        position = float3(SphereData.Radius * sin(polar), SphereData.Radius * cos(polar), 0.0f) + SphereData.Position;
        worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
        lineStream.Append((Pixelinput) worldPosition);
        lineStream.RestartStrip();
    }
}
