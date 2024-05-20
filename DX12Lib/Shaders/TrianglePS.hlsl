
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

struct AmbientDesc
{
    float4 Up;
    float4 Down;
};

struct Tex
{
    bool HasTexture;
};

struct DirectionalLight
{
    float3 direction;
    float3 color;
};

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 Texture : TEXCOORD;
};

ConstantBuffer<Tex> Text : register(b1);
ConstantBuffer<AmbientDesc> Ambient : register(b2);

SamplerState Sampler : register(s0);
Texture2D Texture : register(t1);

float4 CalculateSemiAmbient(float3 normal, float3 color);
float3 CalculateAmbient(DirectionalLight light);
float3 CalculateDiffuse(float3 norm, DirectionalLight light);

[earlydepthstencil]
float4 main(PixelShaderInput IN) : SV_Target
{
    float3 norm = normalize(IN.Normal);
    
    float3 color;
    if (Text.HasTexture)
    {
        float2 uv = IN.Texture;
        uv.y = 1 - uv.y;
        color = Texture.Sample(Sampler, uv);
    }
    else
    {
        color = IN.Color;
    }
    
    DirectionalLight dirLight;
    dirLight.direction = normalize(float3(-0.5, 1, -0.5));
    dirLight.color = float3(1.0, 1.0, 1.0);
    color = (CalculateAmbient(dirLight) + CalculateDiffuse(norm, dirLight)) * color;
    
    return float4(color, 1.0);
}

float4 CalculateSemiAmbient(float3 normal, float3 color)
{
    // Convert from [-1, 1] to [0, 1]
    float up = normal.y * 0.5 + 0.5;
    // Calculate the ambient value
    float4 ambient = Ambient.Down + up * Ambient.Up;

    // Apply the ambient value to the color
    return ambient;
}

float3 CalculateAmbient(DirectionalLight light)
{
    return 0.1 * light.color;
}

float3 CalculateDiffuse(float3 norm, DirectionalLight light)
{
    float3 lightDirection = normalize(light.direction);
    float diffuseFactor = max(dot(norm, lightDirection), 0.0f);
    return light.color * diffuseFactor;
}
