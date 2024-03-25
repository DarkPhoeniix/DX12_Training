
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

struct AmbientDesc
{
    float4 Up;
    float4 Down;
};

ConstantBuffer<AmbientDesc> Ambient : register(b1);

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 Texture : TEXCOORD;
};

SamplerState Sampler : register(s0);
Texture2D Texture : register(t1);

float4 CalculateAmbient(float3 normal, float3 color)
{
    // Convert from [-1, 1] to [0, 1]
    float up = normal.y * 0.5 + 0.5;
    // Calculate the ambient value
    float4 ambient = Ambient.Down + up * Ambient.Up;

    // Apply the ambient value to the color
    return ambient;
}

float4 main(PixelShaderInput IN) : SV_Target
{
    float3 norm = normalize(IN.Normal);
    float2 uv = IN.Texture;
    uv.y = 1 - uv.y;
    //return Texture.Sample(Sampler, uv);
    //return IN.Color;
    return CalculateAmbient(norm, IN.Color.rgb);
}
