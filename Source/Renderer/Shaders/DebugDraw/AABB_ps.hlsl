
struct Pixelinput
{
    float4 position : SV_POSITION;
};

struct ColorData
{
    float4 Color;
};

ConstantBuffer<ColorData> Color : register(b2);

float4 main(Pixelinput input) : SV_Target
{
    return Color.Color;
}
