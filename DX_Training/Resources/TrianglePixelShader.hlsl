
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

struct PixelShaderInput
{
    float4 Color : COLOR;
};

float4 main(PixelShaderInput IN) : SV_Target
{
    return IN.Color;
}
