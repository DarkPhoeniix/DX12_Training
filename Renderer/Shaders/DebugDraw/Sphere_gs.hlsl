
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

static const int NUM_SEGMENTS = 8;

[maxvertexcount(256)]
void main(point Geometryinput input[1], inout LineStream<Pixelinput> lineStream)
{
    for (int i = 0; i < NUM_SEGMENTS; ++i)
    {
        for (int j = 0; j < NUM_SEGMENTS; ++j)
        {
            int pointIndex = i * NUM_SEGMENTS + j;
            
            float polar = PI * (i - 1) / NUM_SEGMENTS;
            float azimuth = 2.0f * PI * j / NUM_SEGMENTS;
            float3 position = SphericalToCartesian(SphereData.Radius, polar, azimuth) + SphereData.Position;
            float4 worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
            lineStream.Append((Pixelinput) worldPosition);
            
            polar = PI * i / NUM_SEGMENTS;
            azimuth = 2.0f * PI * j / NUM_SEGMENTS;
            position = SphericalToCartesian(SphereData.Radius, polar, azimuth) + SphereData.Position;
            worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
            lineStream.Append((Pixelinput) worldPosition);
            lineStream.RestartStrip();
            
            polar = PI * i / NUM_SEGMENTS;
            azimuth = 2.0f * PI * (j - 1) / NUM_SEGMENTS;
            position = SphericalToCartesian(SphereData.Radius, polar, azimuth) + SphereData.Position;
            worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
            lineStream.Append((Pixelinput) worldPosition);
            
            polar = PI * i / NUM_SEGMENTS;
            azimuth = 2.0f * PI * j / NUM_SEGMENTS;
            position = SphericalToCartesian(SphereData.Radius, polar, azimuth) + SphereData.Position;
            worldPosition = mul(float4(position, 1.0f), Instance.ViewProj);
            
            lineStream.Append((Pixelinput) worldPosition);
            lineStream.RestartStrip();
        }
    }
}
