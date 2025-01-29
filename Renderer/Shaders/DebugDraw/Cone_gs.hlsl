
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

struct Cone
{
    float3 Position;
    float Radius;
    float3 Direction;
    float Height;
};

ConstantBuffer<Camera> Instance : register(b0);
ConstantBuffer<Cone> ConeData : register(b1);

float3 SphericalToCartesian(float radius, float polar, float azimuth)
{
    float x = radius * sin(polar) * cos(azimuth);
    float y = radius * cos(polar);
    float z = radius * sin(polar) * sin(azimuth);
    
    return float3(x, y, z);
}

static const int NUM_SEGMENTS = 15;

[maxvertexcount(256)]
void main(point Geometryinput input[1], inout LineStream<Pixelinput> lineStream)
{
    float3 up = float3(0, 1, 0); // Original cone up direction
    float3 newY = normalize(ConeData.Direction);
    float3 newX = normalize(cross(up, newY));
    float3 newZ = cross(newX, newY);

    float3x3 rotationMatrix = float3x3(newX, newY, newZ);
    
    for (int i = 0; i < NUM_SEGMENTS; ++i)
    {
        float3 positionLocal = mul(float3(0.0f, 0.0f, 0.0f), rotationMatrix);
        positionLocal += ConeData.Position;
        float4 position = mul(float4(positionLocal, 1.0f), Instance.ViewProj);
        lineStream.Append((Pixelinput) position);
        
        
        float angle = 2.0f * PI * i / NUM_SEGMENTS;
        positionLocal = float3(ConeData.Radius * sin(angle), ConeData.Height, ConeData.Radius * cos(angle));
        positionLocal = mul(positionLocal, rotationMatrix);
        positionLocal += ConeData.Position;
        position = mul(float4(positionLocal, 1.0f), Instance.ViewProj);
        
        lineStream.Append((Pixelinput) position);
        lineStream.RestartStrip();
        lineStream.Append((Pixelinput) position);
        
        angle = 2.0f * PI * (i - 1) / NUM_SEGMENTS;
        positionLocal = float3(ConeData.Radius * sin(angle), ConeData.Height, ConeData.Radius * cos(angle));
        positionLocal = mul(positionLocal, rotationMatrix);
        positionLocal += ConeData.Position;
        position = mul(float4(positionLocal, 1.0f), Instance.ViewProj);
        
        lineStream.Append((Pixelinput) position);
        lineStream.RestartStrip();
    }
}
