
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
    float4 PositionH : SV_Position;
    float4 Normal : NORMAL;
    float4 Color : COLOR;
};

float3 CalcAmbient(float3 normal, float3 color)
{
    // Convert from [-1, 1] to [0, 1]
    float up = normal.y * 0.5 + 0.5;
    // Calculate the ambient value
    float3 ambient = Ambient.Down + up * Ambient.Up;

    // Apply the ambient value to the color
    return ambient * color;
}

float4 main(PixelShaderInput IN) : SV_Target
{
    // Normalize the input normal
    float3 normal = normalize(IN.Normal);
    
    // Call the helper function and return the value
    return float4(CalcAmbient(normal, IN.Color.rgb), 1.0f);

}
